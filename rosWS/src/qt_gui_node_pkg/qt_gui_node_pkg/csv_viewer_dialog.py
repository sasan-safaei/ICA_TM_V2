
import os
import csv
import ast
import threading
from datetime import datetime

import pandas as pd
import qrcode
from PIL.ImageQt import ImageQt
from PyQt5.QtCore import Qt, QModelIndex
from PyQt5.QtGui import QStandardItemModel, QStandardItem, QPixmap
from PyQt5.QtWidgets import (
    QDialog, QHBoxLayout, QVBoxLayout, QTreeView, QTableView, QFileSystemModel, QHeaderView,
    QPushButton, QMessageBox, QLabel, QListWidget, QAbstractItemView, QDialogButtonBox, QCheckBox
)

from qt_gui_node_pkg.csvToPDF import csv_to_pdf_with_qr
from qt_gui_node_pkg.sendEmail import send_email, send_email_with_retry
# Example usage:
# csv_to_pdf_with_qr("/path/to/your.csv", "/home/ica/PDF/with_qr.pdf")
import os
tm_workspace = os.environ.get("TM_WORKSPACE")
CONFIG_CFG_PATH = os.path.join(tm_workspace, "config.cfg")

class CsvViewerDialog(QDialog):
    def __init__(self, folder_path=os.path.expanduser("~"), printer_name="", parent=None):
        super().__init__(parent)
        self.workspace=folder_path        
        self.printer_name = (printer_name or "").strip()
        self._pending_email_retry_keys = set()
        self.setWindowTitle("CSV Viewer")
        self.setFixedSize(800, 460)
        main_layout = QVBoxLayout(self)
        content_layout = QHBoxLayout()

        # Folder/File view
        self.model = QFileSystemModel()
        self.model.setRootPath(folder_path)
        self.model.setNameFilters(["*.csv"])
        self.model.setNameFilterDisables(False)

        self.tree = QTreeView()
        self.tree.setModel(self.model)
        self.tree.setRootIndex(self.model.index(folder_path))
        self.tree.setColumnWidth(0, 200)
        self.tree.clicked.connect(self.on_file_selected)
        content_layout.addWidget(self.tree, 1)

        # Table view
        self.table = QTableView()
        self.table.setEditTriggers(QTableView.NoEditTriggers)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.table.clicked.connect(self.on_table_row_selected)
        content_layout.addWidget(self.table, 2)

        # QR code label
        self.qr_label = QLabel()
        self.qr_label.setFixedSize(120, 120)
        self.qr_label.setAlignment(Qt.AlignCenter)
        content_layout.addWidget(self.qr_label, 0)

        main_layout.addLayout(content_layout)

        # Buttons at the bottom
        btn_layout = QHBoxLayout()
        self.back_btn = QPushButton("Back")
        self.back_btn.clicked.connect(self.reject)
        btn_layout.addWidget(self.back_btn)

        self.print_all_btn = QPushButton("Send All")
        self.print_all_btn.clicked.connect(self.send_all)
        btn_layout.addWidget(self.print_all_btn)

        self.lasts_print_btn = QPushButton("Send Lasts")
        self.lasts_print_btn.clicked.connect(self.send_lasts)
        btn_layout.addWidget(self.lasts_print_btn)

        self.custom_print_btn = QPushButton("Send Custom")
        self.custom_print_btn.clicked.connect(self.send_custom)
        btn_layout.addWidget(self.custom_print_btn)

        self.print_checkbox = QCheckBox("Print")
        btn_layout.addWidget(self.print_checkbox)

        main_layout.addLayout(btn_layout)

        self.current_csv_path = None
        self.current_df = None

    def sendEmail(self):
        if not self.current_csv_path:
            return
        email_ok, email_msg = self._send_email_with_attachment(self.current_csv_path)
        self._show_action_result(email_ok=email_ok, email_msg=email_msg)

    def on_file_selected(self, index: QModelIndex):
        file_path = self.model.filePath(index)
        if os.path.isfile(file_path) and file_path.lower().endswith('.csv'):
            try:
                expected_columns = ["Num", "EUI-M", "EUI", "Device", "Date", "Time", "Status", "Description"]
                df = self._read_csv_trim_extra(file_path, expected_columns)
                self.current_df = df
                # Columns to hide
                columns_to_hide = ['EUI-M', 'Description']
                # Get visible columns
                visible_columns = [col for col in df.columns if col not in columns_to_hide]
                model = QStandardItemModel(df.shape[0], len(visible_columns))
                model.setHorizontalHeaderLabels(visible_columns)
                for row in range(df.shape[0]):                    
                    for col, col_name in enumerate(visible_columns):                        
                        item = QStandardItem(str(df.at[row, col_name]))
                        model.setItem(row, col, item)
                self.table.setModel(model)
                self.current_csv_path = file_path
                self.qr_label.clear()
                # Show row 2 and its QR code if it exists
                if df.shape[0] > 2 and "EUI" in df.columns:
                    #value = str(df.iloc[2][df.columns.get_loc("EUI")])
                    value = str(df.at[2, "EUI"])
                    self.show_qr_code(value)
                    self.table.selectRow(2)
            except Exception as e:
                model = QStandardItemModel(1, 1)
                model.setItem(0, 0, QStandardItem(f"Error: {e}"))
                self.table.setModel(model)
                self.current_csv_path = None
                self.current_df = None
                self.qr_label.clear()     

    def _read_csv_trim_extra(self, file_path, expected_columns):
        alias_map = {
            "num": ["num"],
            "eui-m": ["eui-m", "euim", "eui_m"],
            "eui": ["eui"],
            "device": ["device"],
            "date": ["date"],
            "time": ["time"],
            "status": ["status", "result"],
            "description": ["description", "desc"],
        }

        required_keys = ["num", "eui-m", "eui", "device", "date", "time"]

        def normalize_name(value):
            return value.strip().lower().replace("_", "-")

        with open(file_path, newline='', encoding='utf-8-sig') as csv_file:
            reader = csv.reader(csv_file)
            rows = list(reader)
        if not rows:
            raise ValueError("CSV is empty")

        header = [h.strip() for h in rows[0]]
        normalized_header = [normalize_name(h) for h in header]

        source_idx_by_key = {}
        for canonical_key, aliases in alias_map.items():
            for alias in aliases:
                if alias in normalized_header:
                    source_idx_by_key[canonical_key] = normalized_header.index(alias)
                    break

        missing_required = [k for k in required_keys if k not in source_idx_by_key]
        if missing_required:
            missing_text = ", ".join(missing_required)
            raise ValueError(f"CSV missing required columns: {missing_text}")

        canonical_keys = ["num", "eui-m", "eui", "device", "date", "time", "status", "description"]
        data_rows = []
        for row in rows[1:]:
            if not row:
                continue
            normalized_row = []
            for key in canonical_keys:
                src_idx = source_idx_by_key.get(key)
                value = row[src_idx].strip() if src_idx is not None and src_idx < len(row) else ""
                normalized_row.append(value)
            data_rows.append(normalized_row)
        return pd.DataFrame(data_rows, columns=expected_columns)
    def on_table_row_selected(self, index: QModelIndex):
        if self.current_df is not None:
            row = index.row()
            if "EUI" in self.current_df.columns:
                value = str(self.current_df.iloc[row][self.current_df.columns.get_loc("EUI")])
                self.show_qr_code(value)
            else:
                self.qr_label.clear()
    def show_qr_code(self, value):
        qr = qrcode.QRCode(box_size=3, border=2)
        qr.add_data(value)
        qr.make(fit=True)
        img = qr.make_image(fill_color="black", back_color="white")
        qt_img = ImageQt(img)
        pixmap = QPixmap.fromImage(qt_img)
        self.qr_label.setPixmap(pixmap.scaled(self.qr_label.size(), Qt.KeepAspectRatio))
    def send_all(self):
        if not self.current_csv_path:
            QMessageBox.warning(self, "No file", "No CSV file selected.")
            return
        # First, create the PDF
        base_name = os.path.splitext(os.path.basename(self.current_csv_path))[0]                
        pdf_folder = self._get_pdf_folder()
        pdf_file_path = os.path.join(pdf_folder, base_name + "_result.pdf")

        csv_to_pdf_with_qr(self.current_csv_path, pdf_file_path, "", "")
        email_ok, email_msg = self._send_email_with_attachment(pdf_file_path)
        print_ok = None
        print_msg = ""
        if self.print_checkbox.isChecked():
            print_ok, print_msg = self._send_to_printer(pdf_file_path)
        self._show_action_result(email_ok=email_ok, email_msg=email_msg, print_ok=print_ok, print_msg=print_msg)

    def send_lasts(self):
        if not self.current_csv_path:
            QMessageBox.warning(self, "No file", "No CSV file selected.")
            return
        #QMessageBox.information(self, "Lasts Print", "Lasts Print functionality not implemented yet.")
        _today = datetime.now().strftime('%Y-%m-%d')
        base_name = os.path.splitext(os.path.basename(self.current_csv_path))[0]
        pdf_folder = self._get_pdf_folder()
        pdf_file_path = os.path.join(pdf_folder, base_name + "_result_" + _today + ".pdf")
        csv_to_pdf_with_qr(self.current_csv_path, pdf_file_path, _today, "")        
        email_ok, email_msg = self._send_email_with_attachment(pdf_file_path)
        print_ok = None
        print_msg = ""
        if self.print_checkbox.isChecked():
            print_ok, print_msg = self._send_to_printer(pdf_file_path)
        self._show_action_result(email_ok=email_ok, email_msg=email_msg, print_ok=print_ok, print_msg=print_msg)

    def send_custom(self):
        if not self.current_csv_path or self.current_df is None:
            QMessageBox.warning(self, "No file", "No CSV file selected.")
            return
        if "Date" not in self.current_df.columns:
            QMessageBox.warning(self, "No Date", "CSV does not contain a Date column.")
            return

        dates = [str(d) for d in self.current_df["Date"].dropna().unique() if str(d).strip()]
        dates = sorted(set(dates))
        if not dates:
            QMessageBox.warning(self, "No Dates", "No dates found in the CSV.")
            return

        selected_dates = self._select_dates(dates)
        if not selected_dates:
            return

        base_name = os.path.splitext(os.path.basename(self.current_csv_path))[0]
        filtered_df = self.current_df[self.current_df["Date"].isin(selected_dates)]
        if filtered_df.empty:
            QMessageBox.warning(self, "No Data", "No rows found for the selected dates.")
            return

        pdf_folder = self._get_pdf_folder()
        temp_csv_path = os.path.join(pdf_folder, "temp_selected_dates.csv")
        filtered_df.to_csv(temp_csv_path, index=False)
        pdf_file_path = os.path.join(pdf_folder, base_name + "_result_selected.pdf")
        csv_to_pdf_with_qr(temp_csv_path, pdf_file_path, "", "")
        email_ok, email_msg = self._send_email_with_attachment(pdf_file_path)
        print_ok = None
        print_msg = ""

        if self.print_checkbox.isChecked():
            print_ok, print_msg = self._send_to_printer(pdf_file_path)
        self._show_action_result(email_ok=email_ok, email_msg=email_msg, print_ok=print_ok, print_msg=print_msg)

    def _send_email_with_attachment(self, attachment_path):
        email_recipients = self._read_email_recipients()
        if not email_recipients:
            return None, "Email skipped: no recipients found in [EMAIL]."
        kwargs = self._read_email_server_config()
        if email_recipients:
            kwargs["receiver_email"] = ", ".join(email_recipients)
        # prefer attachments list
        kwargs["attachments"] = [attachment_path] if attachment_path else []
        # try to include GUI/TM versions from gui_node if available
        try:
            try:
                from . import gui_node as _gn
            except Exception:
                import gui_node as _gn
            kwargs["gui_version"] = getattr(_gn, "GUI_version", None)
            kwargs["tm_version"] = getattr(_gn, "TM_Version", None)
        except Exception:
            pass
        if send_email(**kwargs):
            return True, "Email sent successfully."

        retry_key = (
            kwargs.get("receiver_email", ""),
            kwargs.get("subject", ""),
            tuple(kwargs.get("attachments") or []),
        )
        if retry_key not in self._pending_email_retry_keys:
            self._pending_email_retry_keys.add(retry_key)
            threading.Thread(
                target=self._retry_email_worker,
                args=(retry_key, kwargs),
                daemon=True,
            ).start()
            return (
                False,
                "Email send failed now. Auto-retry scheduled every 2 minute until success.",
            )

        return False, "Email send failed. A 1-minute retry job is already running for this email."

    def _retry_email_worker(self, retry_key, kwargs):
        try:
            send_email_with_retry(
                retry_interval_seconds=120,
                **kwargs,
            )
        finally:
            self._pending_email_retry_keys.discard(retry_key)

    def _send_to_printer(self, pdf_file_path):
        printer_addr = self._get_printer_name()
        if not printer_addr:
            return False, "Print failed: no printer in [PRINTER] or printer.cfg."
        cmd = f'lp -d {printer_addr} {pdf_file_path}'
        result = os.system(cmd)
        if result == 0:
            return True, f"Print job sent to {printer_addr}."
        return False, f"Print failed (printer: {printer_addr})."

    def _show_action_result(self, email_ok=None, email_msg="", print_ok=None, print_msg=""):
        messages = []
        has_error = False

        if email_msg:
            messages.append(email_msg)
            if email_ok is False:
                has_error = True

        if print_msg:
            messages.append(print_msg)
            if print_ok is False:
                has_error = True

        if not messages:
            return

        text = "\n".join(messages)
        if has_error:
            QMessageBox.warning(self, "Send/Print Status", text)
        else:
            QMessageBox.information(self, "Send/Print Status", text)

    def _get_printer_name(self):
        if self.printer_name:
            return self.printer_name
        printer_cfg_path = os.path.join(self.workspace, "printer.cfg")
        try:
            with open(printer_cfg_path, "r", encoding="utf-8") as f:
                return f.read().strip()
        except OSError:
            return ""

    def _select_dates(self, dates):
        dialog = QDialog(self)
        dialog.setWindowTitle("Select Dates")
        layout = QVBoxLayout(dialog)

        list_widget = QListWidget(dialog)
        list_widget.addItems(dates)
        list_widget.setSelectionMode(QAbstractItemView.MultiSelection)
        layout.addWidget(list_widget)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, dialog)
        buttons.accepted.connect(dialog.accept)
        buttons.rejected.connect(dialog.reject)
        layout.addWidget(buttons)

        if dialog.exec_() == QDialog.Accepted:
            return [item.text() for item in list_widget.selectedItems()]
        return []

    def _read_email_recipients(self):
        if not os.path.exists(CONFIG_CFG_PATH):
            return []
        recipients = []
        in_email_section = False
        with open(CONFIG_CFG_PATH, "r", encoding="utf-8") as config_file:
            for raw_line in config_file:
                line = raw_line.strip()
                if not line:
                    continue
                if line.startswith("[") and line.endswith("]"):
                    in_email_section = (line.upper() == "[EMAIL]")
                    continue
                if in_email_section:
                    if line.startswith("["):
                        break
                    address = line.strip().strip("\"")
                    if address:
                        recipients.append(address)
        return recipients

    def _read_email_server_config(self):
        if not os.path.exists(CONFIG_CFG_PATH):
            return {}
        cfg = {}
        allowed_keys = {
            "smtp_server",
            "smtp_port",
            "sender_email",
            "sender_password",
            "receiver_email",
            "subject",
            "body",
            "attachment_path",
            "use_ssl",
        }
        in_email_server = False
        with open(CONFIG_CFG_PATH, "r", encoding="utf-8") as config_file:
            for raw_line in config_file:
                line = raw_line.strip()
                if not line or line.startswith("#"):
                    continue
                if line.startswith("[") and line.endswith("]"):
                    in_email_server = (line.upper() == "[EMAIL-SERVER]")
                    continue
                if not in_email_server:
                    continue
                if "=" not in line:
                    continue
                key, value = [p.strip() for p in line.split("=", 1)]
                if key not in allowed_keys:
                    continue
                value = value.split("#", 1)[0].strip().rstrip(",")
                if not value:
                    continue
                try:
                    cfg[key] = ast.literal_eval(value)
                except (ValueError, SyntaxError):
                    cfg[key] = value
        return cfg

    def _build_email_kwargs(self, email_recipients, attachment_path):
        kwargs = self._read_email_server_config()
        if email_recipients:
            kwargs["receiver_email"] = ", ".join(email_recipients)
        kwargs["attachment_path"] = attachment_path
        return kwargs

    def _get_pdf_folder(self):
        pdf_folder = os.path.join(self.workspace, "pdfFiles")
        os.makedirs(pdf_folder, exist_ok=True)
        return pdf_folder

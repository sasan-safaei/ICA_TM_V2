
import os
import csv
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
from qt_gui_node_pkg.sendEmail import send_email
# Example usage:
# csv_to_pdf_with_qr("/path/to/your.csv", "/home/ica/PDF/with_qr.pdf")

CONFIG_CFG_PATH = "/home/ica/ica_usv_test_result/config.cfg"

class CsvViewerDialog(QDialog):
    def __init__(self, folder_path=os.path.expanduser("~"), parent=None):
        super().__init__(parent)
        self.workspace=folder_path        
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
        # Example usage after printing:
        send_email_with_attachment(
            smtp_server="smtp.gmail.com",
            smtp_port=465,
            smtp_user="your_email@gmail.com",
            smtp_password="your_app_password",
            subject="CSV File",
            body="Here is the CSV file you requested.",
            to_email="sasan.safaei@ica.de",
            attachment_path=self.current_csv_path
        )

    def on_file_selected(self, index: QModelIndex):
        file_path = self.model.filePath(index)
        if os.path.isfile(file_path) and file_path.lower().endswith('.csv'):
            try:
                expected_columns = ["Num", "EUI-M", "EUI", "Device", "Date", "Time", "Status", "Description"]
                df = self._read_csv_trim_extra(file_path, expected_columns)
                if list(df.columns) != expected_columns:
                    QMessageBox.critical(self, "Format Error", "CSV format is not compatible.\nExpected columns:\n" +
                                         ", ".join(expected_columns))
                    self.table.setModel(QStandardItemModel(0, 0))
                    self.current_csv_path = None
                    self.current_df = None
                    self.qr_label.clear()
                    return
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
        with open(file_path, newline='', encoding='utf-8-sig') as csv_file:
            reader = csv.reader(csv_file)
            rows = list(reader)
        if not rows:
            raise ValueError("CSV is empty")
        header = rows[0]
        if len(header) < len(expected_columns):
            raise ValueError("CSV header has too few columns")
        header = header[:len(expected_columns)]
        if header != expected_columns:
            raise ValueError("CSV header does not match expected columns")
        data_rows = []
        for row in rows[1:]:
            if not row:
                continue
            if len(row) < len(expected_columns):
                row += [""] * (len(expected_columns) - len(row))
            else:
                row = row[:len(expected_columns)]
            data_rows.append(row)
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
        email_recipients = self._read_email_recipients()
        if email_recipients:
            send_email(receiver_email=", ".join(email_recipients), attachment_path=pdf_file_path)
        else:
            QMessageBox.warning(self, "Email", "No email recipients found in config.cfg [EMAIL].")
        if self.print_checkbox.isChecked():
            try:
                with open(self.workspace+"printer.cfg", "r") as f:
                    printer_addr = f.read().strip()
            except Exception as e:
                QMessageBox.critical(self, "Printer Config Error", f"Could not read printer.cfg: {e}")
                return
            cmd = f'lp -d {printer_addr} {pdf_file_path}'
            result = os.system(cmd)
            if result == 0:
                QMessageBox.information(self, "Print", "Print job sent successfully.")
            else:
                QMessageBox.critical(self, "Print Error", "Failed to send print job.")

    def send_lasts(self):
        #QMessageBox.information(self, "Lasts Print", "Lasts Print functionality not implemented yet.")
        _today = datetime.now().strftime('%Y-%m-%d')
        base_name = os.path.splitext(os.path.basename(self.current_csv_path))[0]
        pdf_folder = self._get_pdf_folder()
        pdf_file_path = os.path.join(pdf_folder, base_name + "_result_" + _today + ".pdf")
        csv_to_pdf_with_qr(self.current_csv_path, pdf_file_path, _today, "")        
        email_recipients = self._read_email_recipients()
        if email_recipients:
            send_email(receiver_email=", ".join(email_recipients), attachment_path=pdf_file_path)
        else:
            QMessageBox.warning(self, "Email", "No email recipients found in config.cfg [EMAIL].")
        if self.print_checkbox.isChecked():
            try:
                with open(self.workspace+"printer.cfg", "r") as f:
                    printer_addr = f.read().strip()
            except Exception as e:
                QMessageBox.critical(self, "Printer Config Error", f"Could not read printer.cfg: {e}")
                return
            cmd = f'lp -d {printer_addr} {pdf_file_path}'
            result = os.system(cmd)
            if result == 0:
                QMessageBox.information(self, "Print", "Print job sent successfully.")
            else:
                QMessageBox.critical(self, "Print Error", "Failed to send print job.")

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

        email_recipients = self._read_email_recipients()
        if not email_recipients:
            QMessageBox.warning(self, "Email", "No email recipients found in config.cfg [EMAIL].")

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

        if email_recipients:
            send_email(receiver_email=", ".join(email_recipients), attachment_path=pdf_file_path)

        if self.print_checkbox.isChecked():
            try:
                with open(self.workspace + "printer.cfg", "r") as f:
                    printer_addr = f.read().strip()
            except Exception as e:
                QMessageBox.critical(self, "Printer Config Error", f"Could not read printer.cfg: {e}")
                return
            cmd = f'lp -d {printer_addr} {pdf_file_path}'
            result = os.system(cmd)
            if result != 0:
                QMessageBox.critical(self, "Print Error", "Failed to send print job.")
            else:
                QMessageBox.information(self, "Print", "Print job sent successfully.")

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

    def _get_pdf_folder(self):
        pdf_folder = os.path.join(self.workspace, "pdfFiles")
        os.makedirs(pdf_folder, exist_ok=True)
        return pdf_folder

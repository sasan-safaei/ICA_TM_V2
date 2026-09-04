from PyQt5 import QtWidgets, QtGui, QtCore
from qt_gui_node_pkg.ui.py import uiW_iv_show
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from PyQt5 import QtCore
import csv
import os


class IVShowWidget(QtWidgets.QWidget):
    def __init__(self, parent=None, default_csv=None):
        super().__init__(parent)
        self.ui = uiW_iv_show.Ui_Dialog()
        self.ui.setupUi(self)
        self.setWindowFlags(QtCore.Qt.Window)
        self.setWindowModality(QtCore.Qt.ApplicationModal)
        # add Load button next to Back if not present
        #self.btn_load = QtWidgets.QPushButton(self)
        #self.btn_load.setText('Load')
        #self.btn_load.setGeometry(QtCore.QRect(600, 10, 89, 51))
        #self.btn_load.setObjectName('Btn_Load')
        self.qcheck_boxes = [
            self.ui.cBox_L1,
            self.ui.cBox_L2,
            self.ui.cBox_L3,
            self.ui.cBox_L4,
            self.ui.cBox_L5,
            self.ui.cBox_L6,
        ]
        self.ui.Btn_Back.clicked.connect(self.close)
        self.ui.Btn_Load.clicked.connect(self.on_load_clicked)
        # optional 'show all' / 'hide all' buttons (some UI versions)
        try:
            if hasattr(self.ui, 'BtnAllShow'):
                self.ui.BtnAllShow.clicked.connect(lambda: self.set_all_checks(True))
        except Exception:
            pass
        try:
            if hasattr(self.ui, 'BtnAllHide'):
                self.ui.BtnAllHide.clicked.connect(lambda: self.set_all_checks(False))
        except Exception:
            pass

        self.csv_path = default_csv
        if self.csv_path:
            QtCore.QTimer.singleShot(100, self.load_and_plot_default)

        # plotting limits
        self.MaxY_V = 30.0
        self.MaxY_I = 3.0
        self.MaxY_temp = 85.0

    def load_and_plot_default(self):
        if os.path.isfile(self.csv_path):
            self.load_csv(self.csv_path)

    def on_load_clicked(self):
        # use configured storage folder if available
        start_dir = (os.environ.get('TM_WORKSPACE')+os.environ.get('STORE_FOLDER')) or ''
        if start_dir:
            start_dir = os.path.join(start_dir, 'IV')
        # if STORE_FOLDER is relative, try resolving against TM_WORKSPACE
        if start_dir and not os.path.isabs(start_dir):
            tm_ws = (os.environ.get('TM_WORKSPACE')+os.environ.get('STORE_FOLDER')) or ''
            if tm_ws:
                start_dir = os.path.join(tm_ws, start_dir)
        path, _ = QtWidgets.QFileDialog.getOpenFileName(self, 'Open CSV', start_dir, 'CSV Files (*.csv);;All Files (*)')
        if path:
            self.csv_path = path
            self.load_csv(path)

    def load_csv(self, path):
        # read CSV with header: item,Time,Vin,Iin,Vout,Iout,Vcaps,IC_Temp
        xs = []
        series = []  # list of dicts: {name, type, values}
        try:
            with open(path, newline='') as f:
                reader = csv.reader(f)
                rows = list(reader)
            if not rows:
                raise ValueError('Empty CSV')
            header = [h.strip() for h in rows[0]]
            # find Time column index
            try:
                time_idx = next(i for i, h in enumerate(header) if h.lower() == 'time')
            except StopIteration:
                # try common alternative
                time_idx = 1 if len(header) > 1 else 0

            # prepare series for other columns (skip 'item' if present)
            for i, h in enumerate(header):
                if i == time_idx:
                    continue
                if h.strip().lower() == 'item':
                    continue
                name = h
                low = h.strip().lower()
                # explicit mapping for known column names
                if low in ('vin', 'vout', 'vcaps'):
                    stype = 'V'
                elif low in ('iin', 'iout'):
                    stype = 'I'
                elif 'temp' in low or low == 'ic_temp' or low == 'ic_temp':
                    stype = 'T'
                else:
                    # fallback to prefix-based detection
                    if low.startswith('v'):
                        stype = 'V'
                    elif low.startswith('i'):
                        stype = 'I'
                    elif 'temp' in low:
                        stype = 'T'
                    else:
                        stype = 'N'
                series.append({'name': name, 'type': stype, 'values': [], 'index': i})

            # parse data rows
            for row in rows[1:]:
                if len(row) <= time_idx:
                    continue
                try:
                    x = float(row[time_idx])
                except Exception:
                    continue
                xs.append(x)
                for s in series:
                    try:
                        v = float(row[s['index']]) if s['index'] < len(row) else float('nan')
                    except Exception:
                        v = float('nan')
                    s['values'].append(v)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, 'Error', f'Failed reading CSV:\n{e}')
            return

        # store series for later redraw and wire checkboxes
        self.series = series

        if not xs or not self.series:
            QtWidgets.QMessageBox.information(self, 'No Data', 'No numeric data found in CSV')
            return
        # prepare plotting state
        self.xs = xs
        # assign checkbox labels and connect toggles
        colors = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:gray']
        for idx, s in enumerate(self.series):
            if idx < len(self.qcheck_boxes):
                cb = self.qcheck_boxes[idx]
                cb.setText(s['name'])
                cb.setChecked(True)
                # disconnect previous to avoid duplicate
                try:
                    cb.toggled.disconnect()
                except Exception:
                    pass
                cb.toggled.connect(self.redraw_plot)
            s['color'] = colors[idx % len(colors)]

        # initial draw
        self.redraw_plot()
        # install click handler on the graphics view viewport
        try:
            self.ui.graphicsView.viewport().installEventFilter(self)
        except Exception:
            pass

    def set_all_checks(self, state: bool):
        # block signals to avoid redrawing on each checkbox change
        for cb in self.qcheck_boxes:
            try:
                cb.blockSignals(True)
                cb.setChecked(state)
            except Exception:
                pass
            finally:
                try:
                    cb.blockSignals(False)
                except Exception:
                    pass
        # redraw once after bulk change
        self.redraw_plot()

    def eventFilter(self, obj, event):
        if event.type() == QtCore.QEvent.MouseButtonPress and event.buttons() == QtCore.Qt.LeftButton:
            # map click to image pixel coordinates
            try:
                pos = event.pos()
                scene_pt = self.ui.graphicsView.mapToScene(pos)
                rect = self._scene_rect
                if rect.width() == 0 or rect.height() == 0:
                    return False
                rel_x = (scene_pt.x() - rect.x()) / rect.width()
                rel_y = (scene_pt.y() - rect.y()) / rect.height()
                img_w, img_h = self._img_size
                img_x = rel_x * img_w
                img_y = rel_y * img_h
                # find nearest plotted point using scene coordinates
                best = None
                best_d = None
                for p in getattr(self, '_plot_points', []):
                    dx = p['scene_x'] - scene_pt.x()
                    dy = p['scene_y'] - scene_pt.y()
                    d = (dx*dx + dy*dy) ** 0.5
                    if best is None or d < best_d:
                        best = p
                        best_d = d
                # require a close hit (scene pixels)
                if best is not None and best_d is not None and best_d <= 12.0:
                    # remove previous selection graphics
                    try:
                        for it in getattr(self, '_selection_items', []):
                            self._selection_items_scene.removeItem(it)
                    except Exception:
                        pass
                    scene_x = best['scene_x']
                    scene_y = best['scene_y']
                    unit = ''
                    if best.get('type') == 'I':
                        unit = ' A'
                    elif best.get('type') == 'V':
                        unit = ' V'
                    elif best.get('type') == 'T':
                        unit = ' °C'
                    text = f"{best['name']}: {best['value']}{unit}"
                    txt_item = QtWidgets.QGraphicsTextItem(text)
                    txt_item.setDefaultTextColor(QtGui.QColor('black'))
                    # position annotation to the right and slightly above the point
                    txt_item.setPos(scene_x + 10, scene_y - 20)
                    line_item = QtWidgets.QGraphicsLineItem(scene_x, scene_y, scene_x + 10, scene_y - 10)
                    pen = QtGui.QPen(QtGui.QColor('black'))
                    pen.setWidth(1)
                    line_item.setPen(pen)
                    # add to scene and remember for removal
                    scene = self.ui.graphicsView.scene()
                    scene.addItem(txt_item)
                    scene.addItem(line_item)
                    self._selection_items = [txt_item, line_item]
                    self._selection_items_scene = scene
                    # no message box; annotation is shown on the scene
                    return True
            except Exception:
                pass
        return False

    def redraw_plot(self):
        fig = plt.figure(figsize=(8, 4))
        ax_v = fig.add_subplot(111)
        # create two twins for I and Temp
        ax_i = ax_v.twinx()
        ax_t = ax_v.twinx()

        # shift the extra axes to the right side with offsets
        ax_i.spines['right'].set_position(('axes', 1.12))
        ax_t.spines['right'].set_position(('axes', 1.24))
        for a in (ax_i, ax_t):
            a.spines['right'].set_visible(True)

        # set Y labels and ticks on right for all three axes
        ax_v.yaxis.set_label_position('right')
        ax_v.yaxis.set_ticks_position('right')
        ax_i.yaxis.set_label_position('right')
        ax_i.yaxis.set_ticks_position('right')
        ax_t.yaxis.set_label_position('right')
        ax_t.yaxis.set_ticks_position('right')

        # assign limits
        ax_v.set_ylim(0, self.MaxY_V)
        ax_i.set_ylim(0, self.MaxY_I)
        ax_t.set_ylim(0, self.MaxY_temp)

        # annotate each axis with its max value on the right
        #ax_v.text(1.05, 0.98, f"Max={self.MaxY_V}", transform=ax_v.transAxes, ha='left', va='top')
        #ax_i.text(1.18, 0.98, f"Max={self.MaxY_I}", transform=ax_i.transAxes, ha='left', va='top')
        #ax_t.text(1.30, 0.98, f"Max={self.MaxY_temp}", transform=ax_t.transAxes, ha='left', va='top')

        # plotting
        for idx, s in enumerate(self.series):
            # check visibility via checkbox if available
            visible = True
            if idx < len(self.qcheck_boxes):
                visible = self.qcheck_boxes[idx].isChecked()
            if not visible:
                continue
            target_ax = ax_v
            if s['type'] == 'I':
                target_ax = ax_i
            elif s['type'] == 'T':
                target_ax = ax_t
            ln = target_ax.plot(self.xs, s['values'], label=s['name'], color=s.get('color'))

        # store raw data points for later mapping to scene coordinates
        data_pts = []
        for s in self.series:
            for xv, yv in zip(self.xs, s['values']):
                data_pts.append({'x': xv, 'y': yv, 'name': s['name'], 'type': s.get('type'), 'value': yv})
        self._data_points = data_pts

        ax_v.set_xlabel('Time')
        ax_v.set_ylabel('Voltage (V)')
        ax_i.set_ylabel('Current (A)')
        ax_t.set_ylabel('Temp (°C)')
        ax_v.grid(True)
        # do not show legend (user requested no name/color shown on graph)
        # ax.legend()
        img_path = os.path.join(QtCore.QDir.tempPath(), 'iv_show_plot.png')
        fig.tight_layout()
        fig.savefig(img_path)
        plt.close(fig)
        pix = QtGui.QPixmap(img_path)
        scene = QtWidgets.QGraphicsScene(self)
        scene.addPixmap(pix)
        self.ui.graphicsView.setScene(scene)
        self._img_size = (pix.width(), pix.height())
        self._scene_rect = scene.itemsBoundingRect()
        self.ui.graphicsView.fitInView(self._scene_rect, QtCore.Qt.KeepAspectRatio)

        # now compute scene coordinates for each data point (account for image scaling)
        try:
            canvas = FigureCanvas(fig)
            canvas.draw()
            width, height = canvas.get_width_height()
            pts = []
            rect = self._scene_rect
            for p in self._data_points:
                ax_for = ax_v if p['type'] == 'V' else (ax_i if p['type'] == 'I' else ax_t if p['type'] == 'T' else ax_v)
                try:
                    disp = ax_for.transData.transform((p['x'], p['y']))
                    img_x = float(disp[0])
                    img_y = float(height - disp[1])
                    scene_x = rect.x() + (img_x / float(width)) * rect.width()
                    scene_y = rect.y() + (img_y / float(height)) * rect.height()
                    pts.append({'scene_x': scene_x, 'scene_y': scene_y, 'name': p['name'], 'value': p['value'], 'type': p['type']})
                except Exception:
                    pass
            self._plot_points = pts
        except Exception:
            self._plot_points = []


def open_iv_show(parent=None, default_csv=None):
    w = IVShowWidget(parent=parent, default_csv=default_csv)
    w.show()
    return w

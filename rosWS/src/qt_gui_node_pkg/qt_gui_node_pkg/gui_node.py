import os
tm_workspace = os.environ.get("TM_WORKSPACE")
CONFIG_CFG_PATH = os.path.join(tm_workspace, "config.cfg")

from PyQt5 import QtWidgets
from PyQt5.QtCore import QTimer
from PyQt5 import QtCore
from PyQt5.QtCore import QObject
from functools import partial

from qt_gui_node_pkg.csv_viewer_dialog import CsvViewerDialog
from PyQt5.QtCore import QMetaObject, Qt, pyqtSlot
from qt_gui_node_pkg.ui.py.uiW_main import Ui_MainWindow as UiWMain
from qt_gui_node_pkg.ui.py.uiW_testing import Ui_Testing as UiWTesting  # or whatever class name your .py file defines
from qt_gui_node_pkg.ui.py.uiW_ica2308 import Ui_ica2308 as uiWICA2308  # or whatever class name your .py file defines

from std_msgs.msg import Bool
from std_msgs.msg import Int32
from std_msgs.msg import String

import rclpy
from rclpy.node import Node
from my_msgs_pkg.msg import MyMsgTmStsPub
from my_msgs_pkg.msg import MyMsgTmRsuPub
from my_msgs_pkg.msg import MyMsgTmMsgBoxPub

from my_msgs_pkg.msg import MyMsgQtPub
import sys
from PyQt5.QtWidgets import QApplication, QMessageBox
from PyQt5.QtGui import QGuiApplication, QPixmap
#import sys
#from PyQt5.QtWidgets import QApplication, QMainWindow, QMenu, QAction
from qt_gui_node_pkg.gui_code_dialog import CodeDialog  # Import the dialog
try:
    from .sendEmail import send_email
except ImportError:
    from sendEmail import send_email



class GuiManager(Node):
    #last_tm_log_cnt=0
    dongle_selected_text="..."
    dongle_selected_index=0
    last_tm_run_sts=-1
    def __init__(self, app):
        super().__init__('gui_manager')
        # send_email() !!!!!!!!!!!!!!!!!!!!!!!!! hide just for test!!!!!!!!!!!!!!!!!!!!!
        self.app = app
        #self.sub_run_sts = self.create_subscription(Int32, 'tm_run_sts', self.sts_callback, 10)
        #self.sub_run_log = self.create_subscription(String, 'tm_log', partial(self.run_callback,source="tm_log"), 10)
        #self.sub_run_Dongle = self.create_subscription(String, 'tm_Dongle', partial(self.run_callback,source="tm_Dongle"), 10)
        
        self.subscription = self.create_subscription(MyMsgTmRsuPub,'tm_test_result',partial(self.run_callback,source="tm_test_result"),10)
        self.subscription = self.create_subscription(MyMsgTmMsgBoxPub,'tm_msg_box',partial(self.msgbox_callback,source="tm_msg_box"),10)        
        self.subscription = self.create_subscription(MyMsgTmStsPub,'tm_status_result',partial(self.sts_callback,source="tm_status_result"),10)
        
        #self.pub_btn_cmd = self.create_publisher(Int32, 'qt_btn_cmd', 10)
        #self.pub_reg_donglecmd = self.create_publisher(Int32, 'qt_reg_dongle', 10)
        self.myPublish= self.create_publisher(MyMsgQtPub,'qt_pub',10)
        
        # main Window .........................................................
        self.w_main = WMain()
        self.dut_map, self.dut_versions, self.dut_base_list = self._load_dut_config()
        self.w_main.ui.BtnStart.clicked.connect(lambda:self.publish_btn_CMD(1))
        self.w_main.ui.cBoxDongle.addItem("No_Device")
        self.w_main.ui.cBoxDongle.addItems(self.dut_base_list)
                  
        self.w_main.ui.cBoxDongle.currentIndexChanged.connect(self.on_cbox_dongle_changed)
        self.w_main.ui.cBoxVer.currentTextChanged.connect(self.on_cbox_ver_changed)
        self.w_main.ui.actionAbout.triggered.connect(self.show_about_dialog)
        #self.w_main.ui.actionNewGitPass.triggered.connect(self.show_newGitPass)
        self.w_main.ui.actionPrint_EUI_s.triggered.connect(self.show_prinEUI)
        
        


        # testing window ..................................................
        self.w_testing = WTesting()
        self.w_testing.ui.BtnStop.clicked.connect(lambda:self.publish_btn_CMD(2))
        
        # testing window ..................................................
        self.w_ica2308 = WICA2308()
        self.w_ica2308.ui.BtnStop.clicked.connect(lambda:self.publish_btn_CMD(2))
        self.w_ica2308.ui.BtnReRun.clicked.connect(lambda:self.publish_btn_CMD(3))


        self.w_ica2308.show()
        self.w_testing.show()
        self.w_main.show()
    def set_image(self, path):
        pixmap = QPixmap(path)
        if pixmap.isNull():
            return False
        self.w_main.ui.imageLabel.setPixmap(pixmap)
        self.w_main.ui.imageLabel.setScaledContents(True)   # make it fit the label box
        return True

    def _load_dut_config(self, config_name=CONFIG_CFG_PATH):
        dut_map = {}
        dut_versions = {}
        dut_base_list = []
        config_path = os.path.join(os.path.dirname(__file__), config_name)
        if not os.path.isfile(config_path):
            return dut_map, dut_versions, dut_base_list

        in_dut_section = False
        try:
            with open(config_path, "r", encoding="utf-8") as config_file:
                for line in config_file:
                    stripped = line.strip()
                    if not stripped:
                        continue
                    if stripped.startswith("[") and stripped.endswith("]"):
                        in_dut_section = stripped == "[DUT]"
                        continue
                    if not in_dut_section:
                        continue
                    if ":" not in stripped:
                        continue
                    dut_key, dut_value = [part.strip() for part in stripped.split(":", 1)]
                    if "," not in dut_value:
                        continue
                    base, version = [part.strip() for part in dut_value.split(",", 1)]
                    if base not in dut_map:
                        dut_map[base] = {}
                        dut_versions[base] = []
                        dut_base_list.append(base)
                    if version not in dut_map[base]:
                        dut_map[base][version] = dut_key
                        dut_versions[base].append(version)
        except OSError:
            return dut_map, dut_versions, dut_base_list

        return dut_map, dut_versions, dut_base_list

    def _get_selected_dut_key(self):
        base = self.w_main.ui.cBoxDongle.currentText()
        version = self.w_main.ui.cBoxVer.currentText()
        return self.dut_map.get(base, {}).get(version, base)

    def _get_selected_version_float(self):
        text = self.w_main.ui.cBoxVer.currentText()
        try:
            return float(text)
        except (TypeError, ValueError):
            return 0.0

    def on_cbox_dongle_changed(self, index):
        self.dongle_selected_text = self.w_main.ui.cBoxDongle.currentText()
        print(f"New index: {index}, New value: {self.dongle_selected_text}")
        
        self.w_main.ui.cBoxVer.clear()
        versions = self.dut_versions.get(self.dongle_selected_text, [])
        if versions:
            self.w_main.ui.cBoxVer.addItems(versions)
            self.w_main.ui.cBoxVer.setCurrentIndex(self.w_main.ui.cBoxVer.count() - 1)
        #imageName=self.dongle_selected_text+".jpg" #"noImage.jpg"        
        #self.set_image("./uiBuild/"+imageName)
        msg =MyMsgQtPub()
        msg.btn_press = 0
        self.dongle_selected_index = self.w_main.ui.cBoxDongle.currentIndex()
        msg.dongle_sel = self.w_main.ui.cBoxDongle.currentIndex()
        msg.msgbox_press=0
        msg.board_version = self._get_selected_version_float()
        self.myPublish.publish(msg)
        #self.get_logger().info(f"Published qt_pub {msg} ")
    
    def on_cbox_ver_changed(self, text):
        """Handle cBoxVer selection and convert to float for msg.board_ver"""
        if text:
            try:
                #board_ver_float = float(text)
                msg = MyMsgQtPub()
                msg.btn_press = 0
                msg.dongle_sel = self.w_main.ui.cBoxDongle.currentIndex()
                #msg.board_version = board_ver_float
                msg.msgbox_press = 0
                msg.board_version = self._get_selected_version_float()
                self.myPublish.publish(msg)
                print(f"Board version set to: {msg.board_version}")
                image_name = f"{self._get_selected_dut_key()}.jpg"
                if not self.set_image("./uiBuild/" + image_name):
                    self.set_image("./uiBuild/No_Device.jpg")
                
            except ValueError:
                print(f"Could not convert '{text}' to float")
                self.set_image("./uiBuild/No_Device.jpg")
    
    def publish_btn_CMD(self, btnNum):
        msg =MyMsgQtPub()
        msg.btn_press = btnNum
        msg.dongle_sel = self.w_main.ui.cBoxDongle.currentIndex()
        msg.board_version = self._get_selected_version_float()
        msg.msgbox_press=0
        self.myPublish.publish(msg)
        self.get_logger().info(f"Published qt_pub {msg} ")
    def show_msgbox(self, message: str, yes_text: str = "Yes", no_text: str = "No",timeout_ms: int = 5000 ) -> bool:
        """Show a message box with custom text and return True/False."""
        msgbox = QMessageBox()
        msgbox.setWindowTitle("ROS2 Message")
        msgbox.setText(message)

        msgbox.setWindowFlags(msgbox.windowFlags() | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint)
        # Add buttons with custom labels
        if(yes_text!=""): yes_button = msgbox.addButton(yes_text, QMessageBox.YesRole)
        if(no_text!=""): no_button = msgbox.addButton(no_text, QMessageBox.NoRole)

        msgbox.setStyleSheet("""
            QMessageBox {
                background-color: #00AA00;   /* Green background */
            }
            QLabel {
                color: white;                /* Message text color */
                font-size: 22pt;             /* Bigger message text */
            }
            QPushButton {
                background-color: #4C566A;   /* Button background */
                color: white;                /* Button text */
                padding: 8px 16px;           /* Bigger buttons */
                border-radius: 8px;          /* Rounded corners */
                font-size: 24pt;             /* Bigger text */
            }
            QPushButton:hover {
                background-color: #81A1C1;   /* Hover effect */
            }
            """)
        
        msgbox.setMinimumSize(1200, 800)   # width=400px, height=200px
        msgbox.resize(1200, 800) 
        
        # Timer to close after timeout
        def on_timeout():
            if msgbox.isVisible():
                msgbox.done(0)   # 0 = custom return code (timeout)
                self.get_logger().info("!!!! MessageBox closed by timeout !!!")

        QTimer.singleShot(timeout_ms, on_timeout)
        #QTimer.singleShot(0, lambda: print("Event loop is running"))

        result =msgbox.exec_()
        self.get_logger().info(f"msgbox.exec_ ..... {QMessageBox.Rejected}")
        
        #if result == QMessageBox.Rejected:  # timeout or X button
        #    return False
        
        msg = MyMsgQtPub()
        msg.btn_press = 0
        msg.dongle_sel = self.w_main.ui.cBoxDongle.currentIndex() #self._get_selected_dut_key()
        msg.board_version = self._get_selected_version_float()
        if msgbox.clickedButton() == yes_button:
            self.get_logger().info(f"User pressed {yes_text} ")
            msg.msgbox_press=1
            #return True
        else:
            self.get_logger().info(f"User pressed {no_text}")
            msg.msgbox_press=2
            #return False   
        self.myPublish.publish(msg)
    def show_about_dialog(self):
        self.show_msgbox("ICA test machine \n About App....","Ok","")
        #QMessageBox.about(None, "About This App", "This is a sample Qt5 application.\nVersion 1.0")
        #msgbox = QMessageBox()
        #msgbox.setWindowTitle("Test")
        #msgbox.setText("This is a test message.")
        #msgbox.color.
        #msgbox.setStandardButtons(QMessageBox.Ok)
        #msgbox.setWindowFlags(Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint)
        #msgbox.exec_()
    def show_newGitPass(self):
        dialog = CodeDialog()
        dialog.setWindowFlags(dialog.windowFlags() | Qt.Window)
        dialog.code_input.setFocus()
        dialog.exec_()        
    def show_prinEUI(self):
        dlg = CsvViewerDialog(tm_workspace)#("./TestMachine001")
        dlg.exec_()
    @pyqtSlot()
    def scroll_to_bottom(self):
        self.w_testing.ui.TBrowser_msg.verticalScrollBar().setValue(
            self.w_testing.ui.TBrowser_msg.verticalScrollBar().maximum()
        )
    def msgbox_callback(self, msg, source):
        self.get_logger().info(f"!!!!! new msgBox callback:{msg.tm_msg}({msg.tm_btn_yes}/{msg.tm_btn_no}) cnt: {msg.tm_msg_counter} time:{msg.tm_msg_show_time}")
        #self.get_logger().info(f"!!!!! new msgBox callback:{msg.tm_msg}({msg.tm_btn_yes}/{msg.tm_btn_no}) cnt: {msg.tm_msg_counter} time:{msg.tm_msg_show_time}")
        self.show_msgbox(msg.tm_msg,msg.tm_btn_yes,msg.tm_btn_no,(int)(msg.tm_msg_show_time)*1000)            
    def run_callback(self, msg, source):
        def fmt(v):
                return f"{v:.2f}v" if v >= 0 else "--"            
        #if self.last_tm_run_sts==3:                    
        if self.dongle_selected_index==5:#ICA2308
            QMetaObject.invokeMethod( self.w_ica2308.ui.label_Name, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, self.dongle_selected_text))            
            rstring=f"IN: {msg.vin:.1f}V, {msg.in_current:.3f}A"
            QMetaObject.invokeMethod( self.w_ica2308.ui.label_IN_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
            rstring=f"Vcc: {msg.vvcc:.2f}V"
            QMetaObject.invokeMethod( self.w_ica2308.ui.label_T3, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
            rstring=msg.uid_str.replace('\n', '').strip()            
            rstring=rstring.split(']', 1)[-1].strip()
            QMetaObject.invokeMethod( self.w_ica2308.ui.label_T4, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "EID: "+rstring))            
        else:#ICA2407
            QMetaObject.invokeMethod( self.w_ica2308.ui.label_IN_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, ".."))
            QMetaObject.invokeMethod( self.w_ica2308.ui.label_T3, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, ".."))
        
        if source == 'tm_test_result':
            if msg.current_test_no>=1:
                QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_On, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_Off, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                
            if msg.current_test_no>=3:
                QMetaObject.invokeMethod( self.w_testing.ui.label_T3, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
            if msg.current_test_no>=4:
                QMetaObject.invokeMethod(self.w_testing.ui.CBox_UART, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod( self.w_testing.ui.label_T4, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
            if msg.current_test_no>=5:
                QMetaObject.invokeMethod( self.w_testing.ui.label_IN_Data, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod( self.w_testing.ui.label_OUT_Data, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod( self.w_testing.ui.label_Cap_Data, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod( self.w_testing.ui.label_T5_time, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod( self.w_testing.ui.label_allCaps, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
            if msg.current_test_no>=6:
                QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_Dis, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_En, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                QMetaObject.invokeMethod(self.w_testing.ui.CBox_OutSW, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))                
            if msg.current_test_no>=8:
                QMetaObject.invokeMethod( self.w_testing.ui.label_T8_time, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
            if msg.current_test_no>=9:
                QMetaObject.invokeMethod( self.w_testing.ui.label_T9_time, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
            #Line0
            rstring=f"{msg.vin:.1f}V, {msg.in_current:.3f}A"
            QMetaObject.invokeMethod( self.w_testing.ui.label_IN_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
            rstring=f"{msg.vout:.1f}V, {msg.load_current:.3f}A"
            QMetaObject.invokeMethod( self.w_testing.ui.label_OUT_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
            
            rstring=f"{fmt(msg.vcap_bank)}, -"
            QMetaObject.invokeMethod( self.w_testing.ui.label_Cap_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
            #Line1
            tmp = (msg.ar_on == 1)
            QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_On, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, tmp))
            tmp = (msg.ar_off == 1)
            QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_Off, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, tmp))
            tmp = (msg.uart_con == 1)
            QMetaObject.invokeMethod(self.w_testing.ui.CBox_UART, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, tmp))
            rstring=f"Vcc: {msg.vvcc:.2f}V"
            QMetaObject.invokeMethod( self.w_testing.ui.label_T3, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
            #Line2
            rstring=msg.uid_str.replace('\n', '').strip()
            rstring=rstring.split(']', 1)[-1].strip()
            QMetaObject.invokeMethod( self.w_testing.ui.label_T4, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "UID: "+rstring))
            #Line3
            if msg.current_test_no==5:
                rstring=f"{msg.cap_charge_time}Sec"
                QMetaObject.invokeMethod( self.w_testing.ui.label_T5_time, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
            #Line4
            tmp = (msg.flyback_dis == 1)
            QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_Dis, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, tmp))
            tmp = (msg.flyback_en == 1)
            QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_En, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, tmp))
            tmp = (msg.out_sw_off == 1)
            QMetaObject.invokeMethod(self.w_testing.ui.CBox_OutSW, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, tmp))
            
            #Line5
            if msg.current_test_no==8:
                rstring=f"{msg.out_sw_off_time}Sec"
                QMetaObject.invokeMethod( self.w_testing.ui.label_T8_time, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))    
            #Line6
            if msg.current_test_no==9:
                rstring=f"{msg.cap_discharge_time}Sec"
                QMetaObject.invokeMethod( self.w_testing.ui.label_T9_time, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))            
            rstring = f"Caps Voltage: {fmt(msg.vcap1)}, {fmt(msg.vcap2)}, {fmt(msg.vcap3)}, {fmt(msg.vcap4)}"
            #rstring=f"Caps Voltage: {msg.vcap1:.2f}v,{msg.vcap2:.2f}v,{msg.vcap3:.2f}v,{msg.vcap4:.2f}v"
            QMetaObject.invokeMethod( self.w_testing.ui.label_allCaps, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, rstring))
    def sts_callback(self, msg, source):
        #  testing & 2308 log textbox update *******************************************
        if msg.tm_log!="":        
            self.get_logger().info(f"(TM_RunLog) {msg.tm_log}")
            QMetaObject.invokeMethod( self.w_testing.ui.TBrowser_msg, "append", Qt.QueuedConnection, QtCore.Q_ARG(str, msg.tm_log))
            QMetaObject.invokeMethod( self.w_ica2308.ui.TBrowser_msg, "append", Qt.QueuedConnection, QtCore.Q_ARG(str, msg.tm_log))
        # main window *******************************************************************
        if msg.tm_dongle!="":
            QMetaObject.invokeMethod( self.w_main.ui.DUT_Name, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, msg.tm_dongle))
        # Choose which menu to display **************************************************
        if self.last_tm_run_sts!=msg.tm_run_sts:
            self.last_tm_run_sts=msg.tm_run_sts
            match msg.tm_run_sts:
                case 0 |1:
                    self.get_logger().info("Show main Window")
                    QtCore.QMetaObject.invokeMethod( self.w_main, "show", QtCore.Qt.QueuedConnection )
                    QtCore.QMetaObject.invokeMethod( self.w_testing, "hide", QtCore.Qt.QueuedConnection )
                    QtCore.QMetaObject.invokeMethod( self.w_ica2308, "hide", QtCore.Qt.QueuedConnection )                    
                case 2:
                    self.get_logger().info("Show testing window")
                    QtCore.QMetaObject.invokeMethod( self.w_main, "hide", QtCore.Qt.QueuedConnection )
                    QtCore.QMetaObject.invokeMethod( self.w_testing, "show", QtCore.Qt.QueuedConnection )  
                    QtCore.QMetaObject.invokeMethod( self.w_ica2308, "hide", QtCore.Qt.QueuedConnection )    
                    QtCore.QMetaObject.invokeMethod( self.w_testing.ui.TBrowser_msg, "clear", QtCore.Qt.QueuedConnection )
                    
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_On, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_Off, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_UART, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_IN_Data, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_OUT_Data, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_Cap_Data, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T3, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T4, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_Dis, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_En, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_OutSW, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T5_time, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T8_time, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T9_time, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_On, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_AR_Off, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_UART, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_IN_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "In Data"))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_OUT_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "Out Data"))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_Cap_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "Cap Data"))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T3, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "Test3"))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T4, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "Test4"))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_Dis, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_flyback_En, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod(self.w_testing.ui.CBox_OutSW, "setChecked", Qt.QueuedConnection, QtCore.Q_ARG(bool, False))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T5_time, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "Test5"))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T8_time, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "Test8"))
                    QMetaObject.invokeMethod( self.w_testing.ui.label_T9_time, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "Test9"))
                case 3:
                    self.get_logger().info("Show testing ica2308 window")
                    QtCore.QMetaObject.invokeMethod( self.w_main, "hide", QtCore.Qt.QueuedConnection )
                    QtCore.QMetaObject.invokeMethod( self.w_testing, "hide", QtCore.Qt.QueuedConnection )   
                    QtCore.QMetaObject.invokeMethod( self.w_ica2308, "show", QtCore.Qt.QueuedConnection )   
                    QtCore.QMetaObject.invokeMethod( self.w_ica2308.ui.TBrowser_msg, "clear", QtCore.Qt.QueuedConnection )
                    
                    QMetaObject.invokeMethod( self.w_ica2308.ui.label_IN_Data, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                    QMetaObject.invokeMethod( self.w_ica2308.ui.label_T3, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                    QMetaObject.invokeMethod( self.w_ica2308.ui.label_T4, "setEnabled", Qt.QueuedConnection, QtCore.Q_ARG(bool, True))
                    
                    QMetaObject.invokeMethod( self.w_ica2308.ui.label_IN_Data, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "In Data"))
                    QMetaObject.invokeMethod( self.w_ica2308.ui.label_T3, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "VCC"))
                    QMetaObject.invokeMethod( self.w_ica2308.ui.label_T4, "setText", Qt.QueuedConnection, QtCore.Q_ARG(str, "EUI"))                    
                case _:
                    self.get_logger().info("unknown situation")
    def run(self):
        import threading
        from rclpy.executors import SingleThreadedExecutor

        executor = SingleThreadedExecutor()
        executor.add_node(self)

        ros_thread = threading.Thread(target=executor.spin, daemon=True)
        ros_thread.start()

        self.app.exec_()



class WMain(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = UiWMain()
        self.ui.setupUi(self)

class WTesting(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = UiWTesting()
        self.ui.setupUi(self)

class WICA2308(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = uiWICA2308()
        self.ui.setupUi(self)

def main(args=None):
    import sys
    import rclpy
    from rclpy.node import Node

    rclpy.init(args=args)
    app = QtWidgets.QApplication(sys.argv)
    node = GuiManager(app)
    node.run()
    rclpy.shutdown()


if __name__ == '__main__':
    main()

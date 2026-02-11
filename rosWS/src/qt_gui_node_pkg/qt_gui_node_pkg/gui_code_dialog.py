import sys
from PyQt5.QtWidgets import QDialog, QVBoxLayout, QLabel, QLineEdit, QPushButton, QMessageBox, QHBoxLayout

class SimpleKeyboard(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Keyboard")
        self.setFixedSize(800, 400)
        self.is_upper = True  # Track shift state

        self.layout = QVBoxLayout(self)
        self.line_edit = QLineEdit(self)
        self.line_edit.setFixedHeight(50)
        self.layout.addWidget(self.line_edit)

        self.keys_layouts = []
        self.special_row = None
        self.create_keys()
        
    def create_keys(self):
        # Remove old key layouts if any
        for row_layout in getattr(self, 'keys_layouts', []):
            while row_layout.count():
                item = row_layout.takeAt(0)
                widget = item.widget()
                if widget:
                    widget.deleteLater()
            self.layout.removeItem(row_layout)
        self.keys_layouts = []

        # Remove special row if it exists
        if self.special_row is not None:
            while self.special_row.count():
                item = self.special_row.takeAt(0)
                widget = item.widget()
                if widget:
                    widget.deleteLater()
            self.layout.removeItem(self.special_row)
            self.special_row = None

        # Define keys
        if self.is_upper:
            keys = [
                ['1','2','3','4','5','6','7','8','9','0'],
                ['Q','W','E','R','T','Y','U','I','O','P'],
                ['A','S','D','F','G','H','J','K','L','.'],
                ['Z','X','C','V','B','N','M','-','_','@'],
            ]
        else:
            keys = [
                ['1','2','3','4','5','6','7','8','9','0'],
                ['q','w','e','r','t','y','u','i','o','p'],
                ['a','s','d','f','g','h','j','k','l','.'],
                ['z','x','c','v','b','n','m','-','_','@'],
            ]
        # Add keys
        for row in keys:
            row_layout = QHBoxLayout()
            for key in row:
                btn = QPushButton(key)
                btn.setFixedSize(70, 60)
                btn.clicked.connect(lambda _, k=key: self.line_edit.insert(k))
                row_layout.addWidget(btn)
            self.layout.addLayout(row_layout)
            self.keys_layouts.append(row_layout)

        # Add special keys row at the end
        self.special_row = QHBoxLayout()
        self.shift_btn = QPushButton("Shift")
        self.shift_btn.setFixedSize(100, 60)
        self.shift_btn.clicked.connect(self.toggle_shift)
        self.special_row.addWidget(self.shift_btn)

        backspace_btn = QPushButton("Backspace")
        backspace_btn.setFixedSize(150, 60)
        backspace_btn.clicked.connect(self.backspace)
        self.special_row.addWidget(backspace_btn)

        space_btn = QPushButton("Space")
        space_btn.setFixedSize(300, 60)
        space_btn.clicked.connect(lambda: self.line_edit.insert(' '))
        self.special_row.addWidget(space_btn)

        ok_btn = QPushButton("OK", self)
        ok_btn.setFixedSize(150, 60)
        ok_btn.clicked.connect(self.accept)
        self.special_row.addWidget(ok_btn)

        self.layout.addLayout(self.special_row)

    def toggle_shift(self):
        self.is_upper = not self.is_upper
        self.create_keys()

    def backspace(self):
        current = self.line_edit.text()
        self.line_edit.setText(current[:-1])

    def get_text(self):
        return self.line_edit.text()

class CodeDialog(QDialog):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Enter Git Code")
        self.setFixedSize(300, 120)

        # Layout
        layout = QVBoxLayout()

        # Label
        layout.addWidget(QLabel("Please enter your code:"))

        # Textbox
        self.code_input = QLineEdit()
        layout.addWidget(self.code_input)

        # Cancel button
        cancel_btn = QPushButton("Cancel")
        cancel_btn.clicked.connect(self.reject)
        layout.addWidget(cancel_btn)

        # Submit button
        submit_btn = QPushButton("Submit")
        submit_btn.clicked.connect(self.save_code)
        layout.addWidget(submit_btn)

        self.setLayout(layout)

        # Install event filter for keyboard popup
        self.code_input.installEventFilter(self)

    def eventFilter(self, obj, event):
        from PyQt5.QtCore import QEvent
        if obj == self.code_input and event.type() == QEvent.MouseButtonPress:
            keyboard = SimpleKeyboard(self)
            if keyboard.exec_() == QDialog.Accepted:
                self.code_input.setText(keyboard.get_text())
            return True  # Event handled
        return super().eventFilter(obj, event)

    def save_code(self):
        code = self.code_input.text().strip()
        if not code:
            QMessageBox.warning(self, "Error", "Code cannot be empty!")
            return

        # Save to gitPass.txt
        with open("gitPass.txt", "w") as f:
            f.write(code)

        QMessageBox.information(self, "Saved", "Code saved successfully!")
        self.accept()
# ...existing code...
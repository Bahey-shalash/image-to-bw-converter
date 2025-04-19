"""
File: gui_app.py
---------------------------
Description:
    A cross-platform PySide6 GUI frontend for converting color images
    into 1-bit black-and-white PNGs using Floyd–Steinberg error diffusion.
    Uses a C backend via a shared library (libbwconvert.so).

Author: Bahey Shalash
Version: 2.0
Date: 19/04/2025

Dependencies:
    - PySide6
    - libbwconvert.so (compiled from bw_converter.c)

Usage:
    python gui_app.py

Features:
    - Threshold slider (0–255)
    - Invert output toggle
    - Verbose mode toggle
    - Input/Output image preview
"""

import sys
from ctypes import CDLL, c_char_p, c_int
from PySide6.QtCore import Qt
from PySide6.QtGui import QPixmap
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QHBoxLayout,
    QVBoxLayout,
    QFormLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QSlider,
    QCheckBox,
    QFileDialog,
    QGroupBox,
)

# Load the shared library
lib = CDLL("./libbwconvert.so")
lib.convert_image_bw.argtypes = [c_char_p, c_char_p, c_int, c_int, c_int]
lib.convert_image_bw.restype = c_int


class BWConverter(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Image to BW Altium Converter")
        self.setMinimumSize(800, 600)

        # --- File selectors ---
        self.input_edit = QLineEdit()
        btn_in = QPushButton("Browse…")
        btn_in.clicked.connect(self.browse_input)

        self.output_edit = QLineEdit("output.png")
        btn_out = QPushButton("Browse…")
        btn_out.clicked.connect(self.browse_output)

        form = QFormLayout()
        inp_layout = QHBoxLayout()
        inp_layout.addWidget(self.input_edit)
        inp_layout.addWidget(btn_in)
        form.addRow("Input Image:", inp_layout)

        out_layout = QHBoxLayout()
        out_layout.addWidget(self.output_edit)
        out_layout.addWidget(btn_out)
        form.addRow("Output PNG:", out_layout)

        # --- Threshold slider ---
        self.threshold = QSlider(Qt.Horizontal)
        self.threshold.setRange(0, 255)
        self.threshold.setValue(128)
        self.threshold_label = QLabel("128")
        self.threshold.valueChanged.connect(
            lambda v: self.threshold_label.setText(str(v))
        )
        thr_layout = QHBoxLayout()
        thr_layout.addWidget(self.threshold)
        thr_layout.addWidget(self.threshold_label)
        form.addRow("Threshold:", thr_layout)

        # --- Options ---
        self.invert_check = QCheckBox("Invert output")
        self.verbose_check = QCheckBox("Verbose mode")
        form.addRow(self.invert_check)
        form.addRow(self.verbose_check)

        # --- Convert button & status ---
        self.convert_btn = QPushButton("Convert")
        self.convert_btn.clicked.connect(self.convert)
        self.status_label = QLabel("")

        # --- Preview area ---
        preview_group = QGroupBox("Preview")
        prev_layout = QHBoxLayout()
        self.preview_in = QLabel("Input preview")
        self.preview_in.setAlignment(Qt.AlignCenter)
        self.preview_out = QLabel("Output preview")
        self.preview_out.setAlignment(Qt.AlignCenter)
        prev_layout.addWidget(self.preview_in)
        prev_layout.addWidget(self.preview_out)
        preview_group.setLayout(prev_layout)

        # --- Assemble main layout ---
        main_layout = QVBoxLayout()
        main_layout.addLayout(form)
        main_layout.addWidget(self.convert_btn)
        main_layout.addWidget(self.status_label)
        main_layout.addWidget(preview_group)
        self.setLayout(main_layout)

    def browse_input(self):
        path, _ = QFileDialog.getOpenFileName(self, "Select Input Image", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tga *.hdr *.psd *.pic *.ppm *.pgm);;All Files (*)")
        if path:
            self.input_edit.setText(path)
            pix = QPixmap(path).scaled(
                300, 300, Qt.KeepAspectRatio, Qt.SmoothTransformation
            )
            self.preview_in.setPixmap(pix)

    def browse_output(self):
        path, _ = QFileDialog.getSaveFileName(
            self, "Select Output PNG", "output.png", "PNG Files (*.png)"
        )
        if path:
            self.output_edit.setText(path)

    def convert(self):
        in_path = self.input_edit.text().strip()
        out_path = self.output_edit.text().strip()
        if not in_path or not out_path:
            self.status_label.setText("Please choose both input and output.")
            return

        thr = self.threshold.value()
        inv = int(self.invert_check.isChecked())
        verb = int(self.verbose_check.isChecked())

        self.convert_btn.setEnabled(False)
        code = lib.convert_image_bw(
            in_path.encode("utf-8"), out_path.encode("utf-8"), thr, inv, verb
        )
        self.convert_btn.setEnabled(True)

        if code == 0:
            self.status_label.setText("Conversion succeeded ✅")
            pix = QPixmap(out_path).scaled(
                300, 300, Qt.KeepAspectRatio, Qt.SmoothTransformation
            )
            self.preview_out.setPixmap(pix)
        else:
            self.status_label.setText(f"Conversion failed (error {code}) ❌")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = BWConverter()
    win.show()
    sys.exit(app.exec())

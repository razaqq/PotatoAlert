"""
Copyright (c) 2019 razaqq

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QPixmap
from utils.resource_path import resource_path
from gui.custom_widget import CustomWidget
from version import __version__


class AboutMenu(QWidget):
    def __init__(self, parent):
        super().__init__(parent)
        self.layout = QHBoxLayout()
        self.setup_ui()

    def setup_ui(self):
        about_widget = CustomWidget(self)
        about_widget.setObjectName('about_widget')

        about = 'Author: \n' \
                f'Version: \n' \
                'Powered by: \n' \
                'License: '
        about2 = 'http://github.com/razaqq\n' \
                 f'{__version__}\n' \
                 'PyQt5, asyncqt, qtmodern and aiohttp\n' \
                 'MIT'

        main_layout = QVBoxLayout()
        # d.setMinimumSize(400, 150)

        about_layout = QHBoxLayout()
        pix = QPixmap(resource_path('./assets/potato.png'))
        pix = pix.scaled(70, 70)
        img = QLabel()
        img.setPixmap(pix)
        about_layout.addWidget(img, alignment=Qt.Alignment(0))
        font = QFont()
        font.setPointSize(10)
        text1 = QLabel(about)
        text1.setFont(font)
        about_layout.addWidget(text1, alignment=Qt.Alignment(0))
        text2 = QLabel(about2)
        text2.setFont(font)
        about_layout.addWidget(text2, alignment=Qt.Alignment(0))
        about_widget.setLayout(about_layout)
        main_layout.addWidget(about_widget, alignment=Qt.Alignment(0))

        ok_btn = QPushButton('OK')
        # ok_btn.clicked.connect(d.accept)
        ok_btn.setDefault(False)
        ok_btn.setAutoDefault(False)
        ok_btn.setFixedWidth(100)
        main_layout.addWidget(ok_btn, alignment=Qt.AlignCenter)

        about_widget.setLayout(main_layout)
        w = QWidget()
        l = QVBoxLayout()
        l.addStretch()
        l.addWidget(about_widget)
        l.addStretch()
        w.setLayout(l)
        self.layout.addStretch()
        self.layout.addWidget(w)
        self.layout.addStretch()
        self.setLayout(self.layout)

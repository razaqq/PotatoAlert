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


class HelpMenu(QWidget):
    def __init__(self, parent):
        super().__init__(parent)
        self.mw = parent
        self.layout = QHBoxLayout()
        self.setup_ui()

    def setup_ui(self):
        about_widget = CustomWidget(self)
        about_widget.setObjectName('help_widget')

        setup_text = """
        0. Enable replays (if you don't have this already). Its easy to do with <a href=\"https://forum.worldofwarships.eu/topic/109363-tool-replay-settings/\">this tool</a><br>
        1. Go to settings ﻿and choose the replays folder (usually something like <code>C:\\Program Files(x86)\﻿World_of_Warships_EU\\replays\\</code>)<br>
        2. Select the regi﻿on you play in<br>
        3. Start up a match. The tool should automatically notice that you started the game and display the stats
        """

        main_layout = QVBoxLayout()
        help_widget = QWidget()
        help_layout = QVBoxLayout()

        header_font = QFont()
        header_font.setPointSize(15)
        header_font.setBold(True)
        header_label = QLabel('How to setup:')
        header_label.setFont(header_font)
        help_layout.addWidget(header_label, alignment=Qt.AlignLeft)

        font = QFont()
        font.setPointSize(10)
        setup_label = QLabel(setup_text)
        setup_label.setFont(font)
        setup_label.setTextFormat(Qt.RichText)
        setup_label.setTextInteractionFlags(Qt.TextBrowserInteraction)
        setup_label.setOpenExternalLinks(True)
        help_layout.addWidget(setup_label, alignment=Qt.AlignLeft)

        help_widget.setLayout(help_layout)
        main_layout.addWidget(help_widget, alignment=Qt.Alignment(0))

        ok_btn = QPushButton('OK')
        ok_btn.setDefault(False)
        ok_btn.setAutoDefault(False)
        ok_btn.setFixedWidth(100)
        main_layout.addWidget(ok_btn, alignment=Qt.AlignCenter)

        ok_btn.clicked.connect(lambda x: self.mw.switch_tab(0))

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

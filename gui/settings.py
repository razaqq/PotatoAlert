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

from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QFileDialog, QPushButton, QComboBox,
                             QCheckBox, QToolButton)
from PyQt5.QtCore import Qt
from gui.custom_widget import CustomWidget


class SettingsMenu(QWidget):
    def __init__(self, parent):
        self.mw = parent
        self.flags = Qt.WindowFlags()
        super().__init__(parent)
        self.config = parent.config
        self.layout = QHBoxLayout()
        self.setup_ui()

    def setup_ui(self):
        settings_widget = CustomWidget(self)
        settings_widget.setObjectName('settings_widget')

        main_layout = QVBoxLayout()
        main_layout.addStretch()
        main_layout.setContentsMargins(10, 10, 10, 10)
        main_layout.setSpacing(0)

        # API KEY
        row1 = QWidget(flags=self.flags)
        row1_layout = QHBoxLayout()
        row1_layout.setContentsMargins(10, 5, 10, 5)

        api_label = QLabel()
        api_label.setFixedSize(110, 20)
        api_label.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        api_label.setText("Wargaming API Key:")
        row1_layout.addWidget(api_label, alignment=Qt.Alignment(0))

        api_key = QLineEdit()
        api_key.setText(self.config['DEFAULT']['api_key'])
        row1_layout.addWidget(api_key, alignment=Qt.Alignment(0))
        row1.setLayout(row1_layout)
        main_layout.addWidget(row1, alignment=Qt.Alignment(0))

        # REPLAYS FOLDER
        row2 = QWidget(flags=self.flags)
        row2_layout = QHBoxLayout()
        row2_layout.setContentsMargins(10, 5, 10, 5)

        replays_label = QLabel()
        replays_label.setFixedSize(110, 20)
        replays_label.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        replays_label.setText("Replays Folder:")
        row2_layout.addWidget(replays_label, alignment=Qt.Alignment(0))

        replays = QLineEdit()
        replays.setText(self.config['DEFAULT']['replays_folder'])
        row2_layout.addWidget(replays, alignment=Qt.Alignment(0))

        def dir_browser():
            fd = QFileDialog()
            fd.resize(500, 500)
            replays.setText(str(fd.getExistingDirectory(self, "Select Directory")))
        tool_but = QToolButton()
        tool_but.setText("...")
        tool_but.clicked.connect(dir_browser)
        row2_layout.addWidget(tool_but, alignment=Qt.Alignment(0))

        row2.setLayout(row2_layout)
        main_layout.addWidget(row2, alignment=Qt.Alignment(0))

        # REGION
        row3 = QWidget(flags=self.flags)
        row3_layout = QHBoxLayout()
        row3_layout.setContentsMargins(10, 5, 10, 5)

        region_label = QLabel()
        region_label.setFixedSize(110, 20)
        region_label.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        region_label.setText("Region:")
        row3_layout.addWidget(region_label, alignment=Qt.Alignment(0))

        region_picker = QComboBox()
        regions = {'eu': 0,
                   'ru': 1,
                   'na': 2,
                   'asia': 3}
        region_picker.addItems(regions.keys())
        region_picker.setFixedSize(70, 20)
        region_picker.setCurrentIndex(int(regions[self.config['DEFAULT']['region']]))
        row3_layout.addWidget(region_picker, alignment=Qt.Alignment(0))
        row3_layout.addStretch()

        row3.setLayout(row3_layout)
        main_layout.addWidget(row3, alignment=Qt.Alignment(0))

        # GOOGLE ANALYTICS
        row4 = QWidget(flags=self.flags)
        row4_layout = QHBoxLayout()
        row4_layout.setContentsMargins(10, 5, 10, 5)

        info_label = QLabel()
        info_label.setFixedSize(110, 20)
        info_label.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        info_label.setText("Google Analytics:")
        row4_layout.addWidget(info_label, alignment=Qt.Alignment(0))

        ga = QCheckBox('')
        ga.setChecked(self.config['DEFAULT'].getboolean('ga'))
        row4_layout.addWidget(ga, alignment=Qt.Alignment(0))
        row4_layout.addStretch()

        row4.setLayout(row4_layout)
        main_layout.addWidget(row4, alignment=Qt.Alignment(0))

        # BUTTONS
        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(10, 5, 10, 10)

        btn_widget = QWidget(flags=self.flags)
        btn_layout = QHBoxLayout()
        btn_layout.setContentsMargins(0, 0, 0, 5)
        btn_widget.setLayout(btn_layout)

        btn_layout.addStretch()
        ok_btn = QPushButton('Save')
        ok_btn.setDefault(False)
        ok_btn.setAutoDefault(False)
        ok_btn.setFixedWidth(75)
        btn_layout.addWidget(ok_btn)

        cancel_btn = QPushButton('Cancel')
        cancel_btn.setDefault(False)
        cancel_btn.setAutoDefault(False)
        cancel_btn.setFixedWidth(75)
        btn_layout.addWidget(cancel_btn)
        btn_layout.addStretch()

        main_layout.addWidget(btn_widget)
        main_layout.addStretch()

        def update_config():
            self.config['DEFAULT']['replays_folder'] = replays.text()
            self.config['DEFAULT']['api_key'] = api_key.text()
            self.config['DEFAULT']['region'] = \
                [region for region, index in regions.items() if index == region_picker.currentIndex()][0]
            self.config['DEFAULT']['ga'] = 'true' if ga.isChecked() else 'false'

        settings_widget.setLayout(main_layout)

        w = QWidget()
        l = QVBoxLayout()
        l.addStretch()
        l.addWidget(settings_widget)
        l.addStretch()
        w.setLayout(l)

        self.layout.addStretch()
        self.layout.addWidget(w)
        self.layout.addStretch()

        self.setLayout(self.layout)

        def switch_to_table():
            self.mw.menu_bar.settings_entry.btn.setChecked(False)
            self.mw.settings_widget.setVisible(False)
            self.mw.menu_bar.table_entry.btn.setChecked(True)
            self.mw.stats_widget.setVisible(True)

        ok_btn.clicked.connect(switch_to_table)
        cancel_btn.clicked.connect(switch_to_table)
        ok_btn.clicked.connect(update_config)
        ok_btn.clicked.connect(self.config.save)
        ok_btn.clicked.connect(self.mw.pa.set_config_reload_needed)
        ok_btn.clicked.connect(self.mw.pa.run)

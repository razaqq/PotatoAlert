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


import os
import sys
import logging
from assets.qtmodern import styles, windows
from PyQt5.QtWidgets import QApplication, QLabel, QTableWidget, QWidget, QTableWidgetItem, QAbstractItemView,\
     QMainWindow, QHeaderView, QAction, QMessageBox, QComboBox, QDialog, QDialogButtonBox, QLineEdit,\
     QToolButton, QFileDialog, QItemDelegate, QHBoxLayout, QVBoxLayout, QStatusBar
from PyQt5.QtGui import QIcon, QFont, QPixmap, QDesktopServices
from PyQt5.QtCore import QRect, Qt, QUrl, QMetaObject
from utils.config import Config
from utils.logger import Logger
from version import __version__


class MyDelegate(QItemDelegate):
    def __init__(self, parent):
        super().__init__(parent)

    def paint(self, painter, option, index):
        super().paint(painter, option, index)
        if True:
            painter.setPen(index.data(1))
            painter.drawRect(option.rect)


class Label(QLabel):
    def __init__(self, parent=None, text=''):
        super().__init__(parent)
        font = QFont()
        font.setPointSize(15)
        self.setFont(font)
        self.setAlignment(Qt.AlignHCenter | Qt.AlignTop)
        self.setText(text)


class Table(QTableWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.init()
        self.init_headers()

    def init(self):
        self.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.setSelectionMode(QAbstractItemView.NoSelection)
        self.setFocusPolicy(Qt.NoFocus)
        self.setAlternatingRowColors(False)
        self.setMouseTracking(False)
        self.setRowCount(12)
        self.setColumnCount(7)
        self.setSortingEnabled(False)
        # self.setItemDelegate(MyDelegate(self))

    def init_headers(self):
        labels = ['Player', 'Ship', 'Matches', 'Winrate', 'Avg Dmg', 'Matches Ship', 'Winrate Ship']
        for i in range(7):
            item = QTableWidgetItem()
            item.setText(labels[i])
            item.setFont(QFont('Segoe UI', 11))
            self.setHorizontalHeaderItem(i, item)

        self.horizontalHeader().setVisible(True)
        self.verticalHeader().setVisible(False)
        self.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.verticalHeader().setSectionResizeMode(QHeaderView.Stretch)


class MainWindow(QMainWindow):
    def __init__(self):
        self.flags = Qt.WindowFlags()
        super().__init__(flags=self.flags)
        self.central_widget = None
        self.config = Config()
        self.layout = QVBoxLayout()
        self.layout.setContentsMargins(0, 0, 0, 0)
        self.layout.setSpacing(0)
        self.init()
        self.create_table_labels()
        self.left_table, self.right_table = self.create_tables()
        self.log_window, self.logger = self.create_log_window()
        self.create_menubar()
        self.config_reload_needed = False

    def init(self):
        self.setMouseTracking(False)
        self.setTabletTracking(False)
        self.set_size()
        self.setWindowTitle("PotatoAlert")
        icon = QIcon()
        icon.addPixmap(QPixmap(resource_path('./assets/potato.png')), QIcon.Normal, QIcon.Off)
        self.setWindowIcon(icon)
        self.setStatusBar(QStatusBar())
        self.central_widget = QWidget(self, flags=self.flags)
        self.setCentralWidget(self.central_widget)
        self.central_widget.setLayout(self.layout)

    def set_size(self):
        self.resize(1500, 550)  # 580
        # self.setMinimumSize(QSize(1500, 580))  # 520

    def create_tables(self):
        table_widget = QWidget()
        table_layout = QHBoxLayout()
        table_layout.setContentsMargins(10, 0, 10, 0)
        t1 = Table()
        t2 = Table()
        table_layout.addWidget(t1)
        table_layout.addWidget(t2)
        table_widget.setLayout(table_layout)
        self.layout.addWidget(table_widget)
        return t1, t2

    def create_table_labels(self):
        label_widget = QWidget()
        label_layout = QHBoxLayout()
        label_layout.setContentsMargins(0, 0, 0, 0)
        label_layout.setSpacing(0)
        label_layout.addStretch()
        l1 = Label(text='Your Team')
        l1.setFont(QFont('Segoe UI', 16, QFont.Bold))
        label_layout.addWidget(l1)
        label_layout.addStretch()
        label_layout.addStretch()
        l2 = Label(text='Enemy Team')
        l2.setFont(QFont('Segoe UI', 16, QFont.Bold))
        label_layout.addWidget(l2)
        label_layout.addStretch()
        label_widget.setLayout(label_layout)
        self.layout.addWidget(label_widget)

    def create_menubar(self):
        menu = self.menuBar()

        settings_menu = menu.addMenu('Edit')
        settings_button = QAction('Settings', self)
        settings_menu.addAction(settings_button)
        settings_button.triggered.connect(self.create_settings_menu)

        help_menu = menu.addMenu('Help')
        github_button = QAction('View Source on Github', self)
        help_menu.addAction(github_button)
        github_button.triggered.connect(self.open_github)
        about_button = QAction('About', self)
        help_menu.addAction(about_button)
        about_button.triggered.connect(self.open_about)

    def create_log_window(self):
        log_window = QWidget()
        logger = Logger(log_window)
        log_layout = QHBoxLayout()
        log_layout.setContentsMargins(10, 10, 10, 0)
        log_window.setFixedHeight(80)
        logging.getLogger().addHandler(logger)
        logging.getLogger().setLevel(logging.INFO)
        log_layout.addWidget(logger.widget)
        log_layout.addStretch()
        log_window.setLayout(log_layout)
        self.layout.addWidget(log_window)
        return log_window, logger

    def create_settings_menu(self):
        d = QDialog()
        d.setFixedSize(450, 152)  # 400 142
        d.setWindowTitle("Settings")
        d.setWindowIcon(QIcon(resource_path('./assets/settings.ico')))

        bb = QDialogButtonBox(d)
        bb.setGeometry(QRect(10, 110, 430, 32))
        bb.setOrientation(Qt.Horizontal)
        bb.setStandardButtons(QDialogButtonBox.Cancel | QDialogButtonBox.Ok)

        api_key = QLineEdit(d)
        api_key.setGeometry(QRect(120, 10, 320, 20))
        api_key.setText(self.config['DEFAULT']['api_key'])

        l1 = QLabel(d)
        l1.setGeometry(QRect(10, 10, 100, 20))
        l1.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        l1.setText("Wargaming API Key:")

        l3 = QLabel(d)
        l3.setGeometry(QRect(10, 40, 100, 20))
        l3.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        l3.setText("Replays Folder:")

        replays = QLineEdit(d)
        replays.setGeometry(QRect(120, 40, 285, 20))
        replays.setText(self.config['DEFAULT']['replays_folder'])

        def dir_brower():
            fd = QFileDialog()
            fd.resize(500, 500)
            replays.setText(str(fd.getExistingDirectory(self, "Select Directory")))
        t = QToolButton(d)
        t.setGeometry(QRect(415, 40, 25, 20))
        t.setText("...")
        t.clicked.connect(dir_brower)

        l4 = QLabel(d)
        l4.setGeometry(QRect(10, 70, 100, 20))
        l4.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        l4.setText("Region:")

        r = QComboBox(d)
        regions = {'eu': 0,
                   'ru': 1,
                   'na': 2,
                   'asia': 3}
        r.addItems(regions.keys())
        r.setGeometry(QRect(120, 70, 69, 20))
        r.setCurrentIndex(int(regions[self.config['DEFAULT']['region']]))

        def update_config():
            self.config['DEFAULT']['replays_folder'] = replays.text()
            self.config['DEFAULT']['api_key'] = api_key.text()
            self.config['DEFAULT']['region'] = [region for region, index in regions.items() if index == r.currentIndex()][0]

        def flag_config_reload_needed():
            self.config_reload_needed = True

        bb.accepted.connect(d.accept)
        bb.accepted.connect(update_config)
        bb.accepted.connect(self.config.save)
        bb.accepted.connect(flag_config_reload_needed)
        bb.rejected.connect(d.reject)
        QMetaObject.connectSlotsByName(d)

        d.exec_()

    @staticmethod
    def open_github():
        url = QUrl('https://github.com/razaqq/PotatoAlert')
        QDesktopServices.openUrl(url)

    def open_about(self):
        about = 'Author: http://github.com/razaqq\n' \
                f'Version: {__version__}\n' \
                'Powered by: PyQt5, asyncqt, qtmodern and aiohttp\n' \
                'License: MIT'
        QMessageBox.about(self, "About", about)

    @staticmethod
    def notify_update(new_version):
        text = 'There is a new version available! <br>' \
               f'Your Version: {__version__} <br>' \
               f'New Version: {new_version} <br>' \
               'Get it <a href="http://github.com/razaqq/PotatoAlert" style="color: rgb(0,255,0)">HERE</a>'
        q = QMessageBox()
        q.setTextFormat(Qt.RichText)
        q.setText(text)
        q.setWindowTitle('Update available')
        icon = QIcon()
        icon.addPixmap(QPixmap(resource_path('./assets/potato.png')), QIcon.Normal, QIcon.Off)
        q.setWindowIcon(icon)
        q.exec_()
        # q.about(self, "Update available", text)

    def fill_tables(self, players):
        for y in range(12):  # clear the tables before inserting
            for x in range(7):
                self.left_table.setItem(y, x, QTableWidgetItem(''))
                self.right_table.setItem(y, x, QTableWidgetItem(''))

        tables = {1: 0, 2: 0}
        table = None
        y = 0
        for player in players:
            if player.team == 0 or player.team == 1:
                table = self.left_table
                y = tables[1]
                tables[1] += 1
            if player.team == 2:
                table = self.right_table
                y = tables[2]
                tables[2] += 1

            for x in range(len(player.row)):
                size = 10 if x < 2 else 12
                font = QFont("Segoe UI", size, QFont.Bold) if x else QFont("Segoe UI", size)
                item = QTableWidgetItem(player.row[x])
                item.setFont(font)
                if player.background:
                    item.setBackground(player.background)
                if player.colors[x]:
                    item.setForeground(player.colors[x])
                if x > 1:
                    item.setTextAlignment(Qt.AlignCenter)
                table.setItem(y, x, item)
                x += 1


def resource_path(relative_path):
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath('.'), relative_path)


def create_gui():
    import sys
    app = QApplication(sys.argv)
    ui = MainWindow()
    styles.dark(app, resource_path('./assets/style.qss'))
    mw = windows.ModernWindow(ui, resource_path('./assets/frameless.qss'))
    mw.show()
    # app.setStyle('Fusion')
    return app, ui

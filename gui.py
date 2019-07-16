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
from PyQt5.QtWidgets import QApplication, QLabel, QTableWidget, QWidget, QTableWidgetItem, QAbstractItemView,\
     QSizePolicy, QMainWindow, QHeaderView, QAction, QMessageBox, QComboBox, QDialog, QDialogButtonBox, QLineEdit,\
     QToolButton, QFileDialog
from PyQt5.QtGui import QIcon, QFont, QPixmap, QDesktopServices
from PyQt5.QtCore import QRect, Qt, QSize, QFile, QTextStream, QUrl, QMetaObject
from assets.colors import Orange, Purple, Cyan, Pink, LGreen, DGreen, Yellow, Red, White
from config import Config
from logger import Logger
from version import __version__


class Label(QLabel):
    def __init__(self, central_widget, x, y, text):
        super().__init__(central_widget)
        self.setGeometry(QRect(x, y, 200, 30))
        font = QFont()
        font.setPointSize(15)
        self.setFont(font)
        self.setAlignment(Qt.AlignHCenter | Qt.AlignTop)
        self.setText(text)


class Table(QTableWidget):
    def __init__(self, central_widget, x, y):
        super().__init__(central_widget)
        self.init(x, y)
        self.init_headers()

    def init(self, x, y):
        self.setGeometry(QRect(x, y, 730, 430))
        self.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.setSelectionMode(QAbstractItemView.NoSelection)
        self.setFocusPolicy(Qt.NoFocus)
        self.setAlternatingRowColors(True)
        self.setMouseTracking(False)

        self.setRowCount(12)
        self.setColumnCount(7)
        __sortingEnabled = self.isSortingEnabled()
        self.setSortingEnabled(False)
        self.setSortingEnabled(__sortingEnabled)

    def init_headers(self):
        labels = ['Player', 'Ship', 'Matches', 'Winrate', 'Avg Dmg', 'Matches Ship', 'Winrate Ship']
        for i in range(7):
            item = QTableWidgetItem()
            item.setText(labels[i])
            item.setFont(QFont('Segoe UI', 11))
            self.setHorizontalHeaderItem(i, item)

        for x in range(self.columnCount()):
            for y in range(self.rowCount()):
                item = QTableWidgetItem()
                # item.setBackground()
                # item.setMouseTracking(False)
                self.setItem(x, y, item)

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
        self.init()
        self.set_size()
        self.left_table = Table(self.central_widget, 10, 30)
        self.right_table = Table(self.central_widget, 755, 30)
        self.log_window, self.logger = self.create_log_window()
        self.create_table_labels()
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
        self.setAutoFillBackground(False)
        self.central_widget = QWidget(self, flags=self.flags)
        self.setCentralWidget(self.central_widget)
        toggle_stylesheet(int(self.config['DEFAULT']['theme']))
        # QMetaObject.connectSlotsByName(self)

    def set_size(self):
        self.resize(1500, 580)  # 520
        size = QSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        size.setHorizontalStretch(0)
        size.setVerticalStretch(0)
        size.setHeightForWidth(self.sizePolicy().hasHeightForWidth())
        self.setSizePolicy(size)
        self.setMinimumSize(QSize(1500, 580))  # 520
        self.setMaximumSize(QSize(3840, 2160))  # 1500 x 520

    def create_tables(self):
        self.left_table = Table(self.central_widget, 10, 30)
        self.right_table = Table(self.central_widget, 755, 30)

    def create_table_labels(self):
        left_label = Label(self.central_widget, 275, 0, 'Your Team')
        left_label.setFont(QFont('Segoe UI', 16, QFont.Bold))
        right_label = Label(self.central_widget, 1020, 0, 'Enemy Team')
        right_label.setFont(QFont('Segoe UI', 16, QFont.Bold))

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
        if hasattr(self, 'log_window'):
            text = self.logger.widget.toHtml()
            self.logger.widget.hide()
            l = Logger(self.log_window)
            l.re_add_text(text)
            logging.getLogger().addHandler(l)
            logging.getLogger().setLevel(logging.INFO)
            l.widget.show()
            return self.log_window, l
        w = QWidget(self.central_widget)
        w.setGeometry(10, 470, 730, 80)  # 430 + 30
        l = Logger(w)
        logging.getLogger().addHandler(l)
        logging.getLogger().setLevel(logging.INFO)
        w.show()
        return w, l

    def create_settings_menu(self):
        d = QDialog()
        d.setFixedSize(450, 182)  # 400 142
        d.setWindowTitle("Settings")
        d.setWindowIcon(QIcon(resource_path('./assets/settings.ico')))

        bb = QDialogButtonBox(d)
        bb.setGeometry(QRect(10, 140, 430, 32))
        bb.setOrientation(Qt.Horizontal)
        bb.setStandardButtons(QDialogButtonBox.Cancel | QDialogButtonBox.Ok)

        c = QComboBox(d)
        themes = {'default': 0,
                  'light': 1,
                  'dark': 2,
                  'dark2': 3,
                  'dark3': 4}
        c.addItems(themes.keys())
        c.setGeometry(QRect(120, 40, 69, 20))
        c.setCurrentIndex(int(self.config['DEFAULT']['theme']))

        api_key = QLineEdit(d)
        api_key.setGeometry(QRect(120, 10, 320, 20))
        api_key.setText(self.config['DEFAULT']['api_key'])

        l1 = QLabel(d)
        l1.setGeometry(QRect(10, 10, 100, 20))
        l1.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        l1.setText("Wargaming API Key:")

        l2 = QLabel(d)
        l2.setGeometry(QRect(10, 40, 100, 20))
        l2.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        l2.setText("Theme:")

        l3 = QLabel(d)
        l3.setGeometry(QRect(10, 70, 100, 20))
        l3.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        l3.setText("Replays Folder:")

        replays = QLineEdit(d)
        replays.setGeometry(QRect(120, 70, 285, 20))
        replays.setText(self.config['DEFAULT']['replays_folder'])

        def dir_brower():
            fd = QFileDialog()
            fd.resize(500, 500)
            replays.setText(str(fd.getExistingDirectory(self, "Select Directory")))
        t = QToolButton(d)
        t.setGeometry(QRect(415, 70, 25, 20))
        t.setText("...")
        t.clicked.connect(dir_brower)

        l4 = QLabel(d)
        l4.setGeometry(QRect(10, 100, 100, 20))
        l4.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        l4.setText("Region:")

        r = QComboBox(d)
        regions = {'eu': 0,
                   'ru': 1,
                   'na': 2,
                   'asia': 3}
        r.addItems(regions.keys())
        r.setGeometry(QRect(120, 100, 69, 20))
        r.setCurrentIndex(int(regions[self.config['DEFAULT']['region']]))

        def update_config():
            self.config['DEFAULT']['replays_folder'] = replays.text()
            self.config['DEFAULT']['api_key'] = api_key.text()
            self.config['DEFAULT']['theme'] = str(c.currentIndex())
            self.config['DEFAULT']['region'] = [region for region, index in regions.items() if index == r.currentIndex()][0]
            toggle_stylesheet(c.currentIndex())

        def flag_config_reload_needed():
            self.config_reload_needed = True

        bb.accepted.connect(d.accept)
        bb.accepted.connect(update_config)
        bb.accepted.connect(self.config.save)
        bb.accepted.connect(flag_config_reload_needed)
        bb.accepted.connect(self.create_log_window)
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
                'Powered by: PyQt5, asyncqt and aiohttp\n' \
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

            item = QTableWidgetItem(player.player_name)
            item.setFont(QFont("Segoe UI", 10))
            table.setItem(y, 0, item)

            item = QTableWidgetItem(player.ship_name)
            item.setFont(QFont("Segoe UI", 10, QFont.Bold))
            table.setItem(y, 1, item)

            if not player.hidden_profile:
                matches = player.matches
                wr = player.winrate
                avg_dmg = player.avg_dmg

                c = Purple() if matches > 20000 else Cyan() if matches > 14000 else LGreen() if matches > 9000 else \
                    Yellow() if matches > 5000 else Orange() if matches > 2000 else Red()
                item = QTableWidgetItem(str(matches))
                item.setFont(QFont("Segoe UI", 12, QFont.Bold))
                item.setForeground(c)
                item.setTextAlignment(Qt.AlignCenter)
                table.setItem(y, 2, item)

                c = Purple() if wr > 65 else Pink() if wr > 60 else Cyan() if wr > 56 else DGreen() if wr > 54 else \
                    LGreen() if wr > 52 else Yellow() if wr > 49 else Orange() if wr > 47 else Red()
                item = QTableWidgetItem(str(wr))
                item.setFont(QFont("Segoe UI", 12, QFont.Bold))
                item.setForeground(c)
                item.setTextAlignment(Qt.AlignCenter)
                table.setItem(y, 3, item)

                c = Pink() if avg_dmg > 48500 else Cyan() if avg_dmg > 38000 else LGreen() if avg_dmg > 33000 else \
                    Yellow() if avg_dmg > 22000 else Orange() if avg_dmg > 16000 else Red()
                item = QTableWidgetItem(str(avg_dmg))
                item.setForeground(c)
                item.setTextAlignment(Qt.AlignCenter)
                item.setFont(QFont("Segoe UI", 12, QFont.Bold))
                table.setItem(y, 4, item)

                # ship specific stats
                if player.ship_name != 'Error':
                    item = QTableWidgetItem(str(player.matches_ship))
                    item.setFont(QFont("Segoe UI", 12, QFont.Bold))
                    if self.config['DEFAULT']['theme'] in [2, 3, 4]:
                        item.setForeground(White())
                    item.setTextAlignment(Qt.AlignCenter)
                    table.setItem(y, 5, item)

                    wr_ship = player.winrate_ship
                    c = Purple() if wr_ship > 65 else Pink() if wr_ship > 60 else Cyan() if wr_ship > 56 else \
                        DGreen() if wr_ship > 54 else LGreen() if wr_ship > 52 else Yellow() if wr_ship > 49 else \
                        Orange() if wr_ship > 47 else Red()
                    item = QTableWidgetItem(str(wr_ship))
                    item.setFont(QFont("Segoe UI", 12, QFont.Bold))
                    item.setForeground(c)
                    item.setTextAlignment(Qt.AlignCenter)
                    table.setItem(y, 6, item)
                else:
                    table.setItem(y, 5, QTableWidgetItem('Error'))
                    table.setItem(y, 6, QTableWidgetItem('Error'))

                # table.resizeColumnsToContents()
        for y in range(tables[1], 12):
            for x in range(7):
                self.left_table.setItem(y, x, QTableWidgetItem(''))
        for y in range(tables[2], 12):
            for x in range(7):
                self.right_table.setItem(y, x, QTableWidgetItem(''))


def resource_path(relative_path):
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath('.'), relative_path)


def toggle_stylesheet(index):
    styles = {
        0: '',
        1: resource_path('./assets/light.qss'),
        2: resource_path('./assets/dark.qss'),
        3: resource_path('./assets/dark2.qss'),
        4: resource_path('./assets/dark3.qss')

    }

    app = QApplication.instance()
    if app is None:
        raise RuntimeError("No Qt Application found.")

    path = styles[index]

    if path:
        file = QFile(path)
        file.open(QFile.ReadOnly | QFile.Text)
        stream = QTextStream(file)
        app.setStyleSheet(stream.readAll())
    else:
        app.setStyleSheet(path)


def create_gui():
    import sys
    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    # toggle_stylesheet(5)
    ui = MainWindow()
    return app, ui

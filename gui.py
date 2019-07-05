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


from PyQt5.QtWidgets import QApplication, QLabel, QTableWidget, QWidget, QTableWidgetItem, QAbstractItemView,\
     QSizePolicy, QMainWindow, QHeaderView, QAction, QMessageBox, QComboBox
from PyQt5.QtGui import QIcon, QFont, QPixmap, QDesktopServices
from PyQt5.QtCore import QRect, Qt, QSize, QFile, QTextStream, QUrl
from assets.colors import Orange, Purple, Cyan, Pink, LGreen, DGreen, Yellow, Red, White, Black
from pyqtconfig import ConfigManager
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

        # TODO ???
        # item = QtWidgets.QTableWidgetItem()
        # self.setItem(0, 5, item)

        self.horizontalHeader().setVisible(True)
        # self.horizontalHeader().setDefaultSectionSize(104)
        # self.horizontalHeader().setHighlightSections(True)
        # self.horizontalHeader().setMinimumSectionSize(49)
        self.verticalHeader().setVisible(False)

        self.horizontalHeader().setSectionResizeMode(0, QHeaderView.Stretch)
        self.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeToContents)
        self.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeToContents)


class MainWindow(QMainWindow):
    def __init__(self):
        self.flags = Qt.WindowFlags()
        super().__init__(flags=self.flags)
        self.central_widget = None
        self.init()
        self.set_size()
        self.left_table = Table(self.central_widget, 10, 30)
        self.right_table = Table(self.central_widget, 755, 30)
        self.create_table_labels()
        self.create_menubar()
        self.config = ConfigManager()
        self.config.set_defaults({
            'replays_folder': 'C:/',
            'api_key': '123',
            'region': 'eu',
            'theme': 0,
        })

    def init(self):
        self.setObjectName("MainWindow")
        self.setMouseTracking(False)
        self.setTabletTracking(False)
        self.set_size()
        self.setWindowTitle("PotatoAlert")
        icon = QIcon()
        icon.addPixmap(QPixmap("assets/potato.png"), QIcon.Normal, QIcon.Off)
        self.setWindowIcon(icon)
        self.setAutoFillBackground(False)

        self.central_widget = QWidget(self, flags=self.flags)
        self.setCentralWidget(self.central_widget)
        # QMetaObject.connectSlotsByName(self)

    def set_status_bar(self, text):
        self.statusBar().showMessage(text)

    def set_size(self):
        self.resize(1500, 520)
        size = QSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        size.setHorizontalStretch(0)
        size.setVerticalStretch(0)
        size.setHeightForWidth(self.sizePolicy().hasHeightForWidth())
        self.setSizePolicy(size)
        self.setMinimumSize(QSize(1500, 520))
        self.setMaximumSize(QSize(1500, 520))

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

    def create_settings_menu(self):
        comboBox = QComboBox()
        themes = {'default': 0,
                  'dark': 1,
                  'light': 2}
        comboBox.addItems(themes.keys())
        self.config.add_handler('combo', comboBox, mapper=themes)

    @staticmethod
    def open_github():
        url = QUrl('https://github.com/razaqq/PotatoAlert')
        QDesktopServices.openUrl(url)

    def open_about(self):
        about = 'Author: http://github.com/razaqq\n' \
                f'Version: {__version__}\n' \
                'Powered by: PyQt5, asyncqt and requests\n' \
                'License: MIT'
        QMessageBox.about(self, "About", about)
        # QLabel('test123')

    def fill_tables(self, players):
        tables = {1: 0, 2: 0}
        table = None
        y = 0
        # carrier, battleship, cruiser, destroyer

        players = sorted(players, key=lambda x: (x.ship['type'], x.ship['tier']), reverse=True)
        for i in range(len(players)):
            player = players[i]
            if player.relation == 0 or player.relation == 1:
                table = self.left_table
                y = tables[1]
                tables[1] += 1
            if player.relation == 2:
                table = self.right_table
                y = tables[2]
                tables[2] += 1

            item = QTableWidgetItem(f'[{player.clan}] {player.name}' if player.clan else player.name)
            item.setFont(QFont("Segoe UI", 10))
            table.setItem(y, 0, item)

            ship_name = 'P. E. Friedrich' if player.ship['name'] == 'Prinz Eitel Friedrich' else \
                'F. der Große' if player.ship['name'] == 'Friedrich der Große' else \
                'Admiral Graf Spee' if player.ship['name'] == 'A. Graf Spee' else \
                'Okt. Revolutsiya' if player.ship['name'] == 'Oktyabrskaya Revolutsiya' else \
                'HSF A. Graf Spee' if player.ship['name'] == 'HSF Admiral Graf Spee' else \
                'J. de la Gravière' if player.ship['name'] == 'Jurien de la Gravière' else player.ship['name']
            item = QTableWidgetItem(ship_name)
            item.setFont(QFont("Segoe UI", 10, QFont.Bold))
            table.setItem(y, 1, item)

            if not player.hidden_profile:
                # overall stats
                battles = player.stats['battles']
                wr = round(player.stats['wins'] / battles * 100, 1)
                avg_dmg = int(player.stats['damage_dealt'] / battles)

                c = Purple() if battles > 20000 else Cyan() if battles > 14000 else LGreen() if battles > 9000 else \
                    Yellow() if battles > 5000 else Orange() if battles > 2000 else Red()
                item = QTableWidgetItem(str(battles))
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
                battles_ship = player.ship_stats['battles']
                wr_ship = round(player.ship_stats['wins'] / battles_ship * 100, 1)
                avg_dmg_ship = int(player.ship_stats['damage_dealt'] / battles_ship)

                item = QTableWidgetItem(str(battles_ship))
                item.setFont(QFont("Segoe UI", 12, QFont.Bold))
                if self.config.get('theme') == 1:
                    item.setForeground(White())
                item.setTextAlignment(Qt.AlignCenter)
                table.setItem(y, 5, item)

                c = Purple() if wr_ship > 65 else Pink() if wr_ship > 60 else Cyan() if wr_ship > 56 else \
                    DGreen() if wr_ship > 54 else LGreen() if wr_ship > 52 else Yellow() if wr_ship > 49 else \
                    Orange() if wr_ship > 47 else Red()
                item = QTableWidgetItem(str(wr_ship))
                item.setFont(QFont("Segoe UI", 12, QFont.Bold))
                item.setForeground(c)
                item.setTextAlignment(Qt.AlignCenter)
                table.setItem(y, 6, item)


def toggle_stylesheet(path=''):
    app = QApplication.instance()
    if app is None:
        raise RuntimeError("No Qt Application found.")

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
    # toggle_stylesheet('assets/dark.qss')
    ui = MainWindow()
    return app, ui

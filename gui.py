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
from assets.qtmodern import styles, windows
from PyQt5.QtWidgets import QApplication, QLabel, QTableWidget, QWidget, QTableWidgetItem, QAbstractItemView,\
     QMainWindow, QHeaderView, QAction, QMessageBox, QComboBox, QDialogButtonBox, QLineEdit, QSizePolicy, \
     QToolButton, QFileDialog, QHBoxLayout, QVBoxLayout, QStatusBar, QCheckBox
from PyQt5.QtGui import QIcon, QFont, QPixmap, QDesktopServices, QMovie
from PyQt5.QtCore import Qt, QUrl, QSize
from utils.dcs import Team
from utils.config import Config
from version import __version__


class MatchInfo:
    def __init__(self, main_layout):
        self.flags = Qt.WindowFlags()
        self.widget = QWidget(flags=self.flags)
        self.layout = QHBoxLayout()
        self.layout.setContentsMargins(20, 5, 20, 0)
        self.layout.setSpacing(20)
        self.widget.setLayout(self.layout)
        self.map = Label(text='Map: ', size=10)
        self.scenario = Label(text='Scenario: ', size=10)
        self.layout.addWidget(self.map, alignment=Qt.Alignment(0))
        self.layout.addWidget(self.scenario, alignment=Qt.Alignment(0))
        self.layout.addStretch()
        main_layout.addWidget(self.widget)

    def update_text(self, map_name, scenario, players_per_team):
        self.map.setText(f'Map: {map_name} ({players_per_team}vs{players_per_team})')
        self.scenario.setText(f'Scenario: {scenario}')


class TeamStats:
    def __init__(self, main_layout):
        self.flags = Qt.WindowFlags()
        widget = QWidget(flags=self.flags)
        layout = QHBoxLayout()
        layout.setContentsMargins(10, 0, 10, 0)
        layout.setSpacing(10)
        widget.setLayout(layout)
        # widget.setStyleSheet('border-style: solid; border-width: 0.5px; border-color: red;')

        self.left_widget = QWidget(flags=self.flags)
        self.left_layout = QHBoxLayout()
        self.left_layout.setContentsMargins(10, 0, 10, 0)
        self.left_layout.setSpacing(20)
        self.left_widget.setLayout(self.left_layout)
        layout.addWidget(self.left_widget, alignment=Qt.Alignment(0))

        self.right_widget = QWidget(flags=self.flags)
        self.right_layout = QHBoxLayout()
        self.right_layout.setContentsMargins(10, 0, 10, 0)
        self.right_layout.setSpacing(20)
        self.right_widget.setLayout(self.right_layout)
        layout.addWidget(self.right_widget, alignment=Qt.Alignment(0))

        self.t1_wr = Label(text='', size=10)
        self.t1_dmg = Label(text='', size=10)
        self.t2_wr = Label(text='', size=10)
        self.t2_dmg = Label(text='', size=10)
        self.t1_server = Label(text='', size=10)
        self.t2_server = Label(text='', size=10)
        self.t1_server_label = Label(text='Server: ', size=10)
        self.t2_server_label = Label(text='Server: ', size=10)

        self.t1_clan_tag = Label(text='', size=10)
        self.t1_clan_name = Label(text='', size=10)
        self.t2_clan_tag = Label(text='', size=10)
        self.t2_clan_name = Label(text='', size=10)

        widgets = [QWidget(flags=self.flags) for _ in range(8)]

        layouts = [QHBoxLayout() for _ in range(8)]
        for layout in layouts:
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(0)

        for i in range(8):
            widgets[i].setLayout(layouts[i])

            if i == 2 or i == 3:
                self.left_layout.addStretch()

            if i == 6 or i == 7:
                self.right_layout.addStretch()

            if i < 4:
                self.left_layout.addWidget(widgets[i], alignment=Qt.Alignment(0))
            else:
                self.right_layout.addWidget(widgets[i], alignment=Qt.Alignment(0))

        layouts[0].addWidget(Label(text='WR: ', size=10), alignment=Qt.AlignRight)
        layouts[0].addWidget(self.t1_wr, alignment=Qt.AlignLeft)
        layouts[1].addWidget(Label(text='DMG: ', size=10), alignment=Qt.AlignRight)
        layouts[1].addWidget(self.t1_dmg, alignment=Qt.AlignLeft)
        layouts[3].addWidget(self.t1_server_label, alignment=Qt.AlignRight)
        layouts[3].addWidget(self.t1_server, alignment=Qt.AlignLeft)
        layouts[2].addWidget(self.t1_clan_tag, alignment=Qt.AlignRight)
        layouts[2].addWidget(self.t1_clan_name, alignment=Qt.AlignLeft)

        layouts[4].addWidget(Label(text='WR: ', size=10), alignment=Qt.AlignRight)
        layouts[4].addWidget(self.t2_wr, alignment=Qt.AlignLeft)
        layouts[5].addWidget(Label(text='DMG: ', size=10), alignment=Qt.AlignRight)
        layouts[5].addWidget(self.t2_dmg, alignment=Qt.AlignLeft)
        layouts[7].addWidget(self.t2_server_label, alignment=Qt.AlignRight)
        layouts[7].addWidget(self.t2_server, alignment=Qt.AlignLeft)
        layouts[6].addWidget(self.t2_clan_tag, alignment=Qt.AlignRight)
        layouts[6].addWidget(self.t2_clan_name, alignment=Qt.AlignLeft)

        main_layout.addWidget(widget)
        self.update_avg(Team(), Team())
        self.update_servers()

    def update_avg(self, team1: Team, team2: Team):
        self.t1_wr.setText(f'{team1.winrate}%')
        self.t1_dmg.setText(f'{team1.avg_dmg}')
        self.t2_wr.setText(f'{team2.winrate}%')
        self.t2_dmg.setText(f'{team2.avg_dmg}')

        self.t1_wr.setStyleSheet(f"color: {team1.winrate_c.name()}")
        self.t1_dmg.setStyleSheet(f"color: {team1.avg_dmg_c.name()}")
        self.t2_wr.setStyleSheet(f"color: {team2.winrate_c.name()}")
        self.t2_dmg.setStyleSheet(f"color: {team2.avg_dmg_c.name()}")

    def update_servers(self, t1_server=None, t2_server=None):
        if t1_server and t2_server:
            self.t1_server.show(), self.t2_server.show()
            self.t1_server_label.show(), self.t2_server_label.show()
            self.t1_server.setText(f'{t1_server}')
            self.t2_server.setText(f'{t2_server}')
        else:
            self.t1_server.hide(), self.t2_server.hide()
            self.t1_server_label.hide(), self.t2_server_label.hide()

    def update_clans(self, c1=None, c2=None):
        if c1 and c2:
            self.t1_clan_name.show(), self.t2_clan_name.show()
            self.t1_clan_tag.show(), self.t2_clan_tag.show()
            self.t1_clan_name.setText(c1[0])
            self.t2_clan_name.setText(c2[0])
            self.t1_clan_tag.setText(f'[{c1[1]}] ')
            self.t2_clan_tag.setText(f'[{c2[1]}] ')
            self.t1_clan_tag.setStyleSheet(f"color: {c1[2]}")
            self.t2_clan_tag.setStyleSheet(f"color: {c2[2]}")
        else:
            self.t1_clan_tag.hide(), self.t2_clan_tag.hide()
            self.t1_clan_name.hide(), self.t2_clan_name.hide()


class Label(QLabel):
    def __init__(self, parent=None, text='', size=16, align=None, bold=True):
        super().__init__(parent)
        if bold:
            self.setFont(QFont('Segoe UI', size, QFont.Bold))
        else:
            self.setFont(QFont('Segoe UI', size))
        if align:
            self.setAlignment(align)
        else:
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
        self.setColumnCount(8)
        self.setSortingEnabled(False)
        self.setContentsMargins(0, 0, 0, 0)

    def init_headers(self):
        labels = ['Player', 'Ship', 'Matches', 'Winrate', 'Avg DMG', 'M Ship', 'WR Ship', 'DMG Ship']
        for i in range(len(labels)):
            item = QTableWidgetItem()
            item.setText(labels[i])
            item.setFont(QFont('Segoe UI', 11))
            self.setHorizontalHeaderItem(i, item)

        headers = QHeaderView(Qt.Horizontal, self)
        self.setHorizontalHeader(headers)
        headers.setSectionResizeMode(QHeaderView.Stretch)
        headers.setSectionResizeMode(1, QHeaderView.ResizeToContents)
        headers.setSectionResizeMode(0, QHeaderView.ResizeToContents)

        self.horizontalHeader().setVisible(True)
        self.verticalHeader().setVisible(False)
        # self.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.verticalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.resizeColumnsToContents()


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
        self.status_icon, self.status_text = None, None
        self.update_status(1, 'Ready')
        self.create_table_labels()
        self.left_table, self.right_table = self.create_tables()
        self.create_menubar()
        self.team_stats = TeamStats(self.layout)
        if self.config['DEFAULT'].getboolean('additional_info'):
            self.match_info = MatchInfo(self.layout)
        self.config_reload_needed = False
        self.mw = None

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
        self.move(self.config.getint('DEFAULT', 'windowx'), self.config.getint('DEFAULT', 'windowy'))
        self.resize(self.config.getint('DEFAULT', 'windoww'), self.config.getint('DEFAULT', 'windowh'))

    def create_tables(self):
        table_widget = QWidget(flags=self.flags)
        table_layout = QHBoxLayout()
        table_layout.setContentsMargins(10, 0, 10, 0)
        table_layout.setSpacing(10)
        t1 = Table()
        t2 = Table()
        table_layout.addWidget(t1, alignment=Qt.Alignment(0))
        table_layout.addWidget(t2, alignment=Qt.Alignment(0))
        table_widget.setLayout(table_layout)
        self.layout.addWidget(table_widget, alignment=Qt.Alignment(0))
        return t1, t2

    def create_table_labels(self):
        label_widget = QWidget(flags=self.flags)
        label_layout = QHBoxLayout()
        label_layout.setContentsMargins(10, 0, 10, 0)
        label_layout.setSpacing(10)
        label_widget.setLayout(label_layout)

        left_layout = QHBoxLayout()
        left_layout.setContentsMargins(0, 0, 0, 0)
        left_layout.setSpacing(0)
        left_widget = QWidget(flags=self.flags)
        # left_widget.setStyleSheet('border-style: solid; border-width: 0.5px; border-color: red;')
        left_widget.setLayout(left_layout)

        status = QWidget(flags=self.flags)
        status.setFixedWidth(130)
        status_layout = QHBoxLayout()
        status_layout.setContentsMargins(0, 0, 0, 0)
        status_layout.setSpacing(0)
        status.setLayout(status_layout)
        status_layout.addWidget(self.status_icon, alignment=Qt.Alignment(0))
        status_layout.addSpacing(5)
        status_layout.addWidget(self.status_text, alignment=Qt.Alignment(0))
        status_layout.addStretch()

        left_layout.addWidget(status, alignment=Qt.Alignment(0))
        left_layout.addStretch()
        left_layout.addWidget(Label(text='Your Team'), alignment=Qt.Alignment(0))
        left_layout.addStretch()
        dummy = QWidget(flags=self.flags)
        dummy.setFixedWidth(130)
        left_layout.addWidget(dummy, alignment=Qt.Alignment(0))

        right_layout = QHBoxLayout()
        right_layout.setContentsMargins(0, 0, 0, 0)
        right_layout.setSpacing(0)
        right_widget = QWidget(flags=self.flags)
        right_widget.setLayout(right_layout)
        right_layout.addStretch()
        right_layout.addWidget(Label(text='Enemy Team'), alignment=Qt.Alignment(0))
        right_layout.addStretch()

        label_layout.addWidget(left_widget, alignment=Qt.Alignment(0))
        label_layout.addWidget(right_widget, alignment=Qt.Alignment(0))
        self.layout.addWidget(label_widget, alignment=Qt.Alignment(0))

    def create_menubar(self):
        def open_github():
            url = QUrl('https://github.com/razaqq/PotatoAlert')
            QDesktopServices.openUrl(url)

        def open_about():
            about = 'Author: \n' \
                    f'Version: \n' \
                    'Powered by: \n' \
                    'License: '
            about2 = 'http://github.com/razaqq\n' \
                     f'{__version__}\n' \
                     'PyQt5, asyncqt, qtmodern and aiohttp\n' \
                     'MIT'
            d = windows.ModernDialog(resource_path('./assets/frameless.qss'), parent=self, hide_window_buttons=True)
            main_layout = QVBoxLayout()
            d.setWindowTitle('About')
            # d.windowContent.setStyleSheet('border-style: solid; border-width: 0.5px; border-color: red;')
            d.setMinimumSize(400, 150)

            about_widget = QWidget(flags=self.flags)
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

            button_box = QDialogButtonBox()
            button_box.setOrientation(Qt.Horizontal)
            button_box.setStandardButtons(QDialogButtonBox.Ok)
            button_box.setCenterButtons(True)
            button_box.accepted.connect(d.accept)
            main_layout.addWidget(button_box, alignment=Qt.Alignment(0))

            d.windowContent.setLayout(main_layout)
            d.exec()

        def open_logs():
            QDesktopServices.openUrl(QUrl.fromLocalFile(self.config.config_path))

        menu = self.menuBar()

        settings_menu = menu.addMenu('Edit')
        settings_button = QAction('Settings', self)
        settings_menu.addAction(settings_button)
        settings_button.triggered.connect(self.create_settings_menu)

        help_menu = menu.addMenu('Help')
        github_button = QAction('View Source on Github', self)
        help_menu.addAction(github_button)
        github_button.triggered.connect(open_github)
        logs_button = QAction('Open Logs', self)
        help_menu.addAction(logs_button)
        logs_button.triggered.connect(open_logs)
        about_button = QAction('About', self)
        help_menu.addAction(about_button)
        about_button.triggered.connect(open_about)

    def update_status(self, status=1, text=''):
        if not self.status_icon or not self.status_text:
            self.status_icon = QLabel()
            self.status_icon.setScaledContents(True)
            self.status_icon.setFixedHeight(20)
            self.status_icon.setFixedWidth(20)
            self.status_text = QLabel('')
            self.status_text.setAlignment(Qt.AlignCenter)
            self.status_text.setStyleSheet('font-size: 10px;')
        if status == 1:  # waiting for start/ready
            pix = QPixmap(resource_path('assets/done.png'))
            self.status_icon.setPixmap(pix)
            self.status_text.setText(text)
        elif status == 2:  # loading
            if not self.status_icon.movie():
                movie = QMovie(resource_path('assets/loading.gif'))
                movie.setSpeed(1000)
                movie.setScaledSize(QSize(20, 20))
                movie.start()
                self.status_icon.setMovie(movie)
            self.status_text.setText(text)
        elif status == 3:  # error
            pix = QPixmap(resource_path('assets/error.png'))
            self.status_icon.setPixmap(pix)
            self.status_text.setText(text)

    def create_settings_menu(self):
        mw = windows.ModernDialog(resource_path('./assets/frameless.qss'), hide_window_buttons=True, parent=self)
        mw.setWindowTitle('Settings')
        mw.setMinimumSize(450, 150)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
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

        def dir_brower():
            fd = QFileDialog()
            fd.resize(500, 500)
            replays.setText(str(fd.getExistingDirectory(self, "Select Directory")))
        tool_but = QToolButton()
        tool_but.setText("...")
        tool_but.clicked.connect(dir_brower)
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

        # ADDITIONAL INFO
        row4 = QWidget(flags=self.flags)
        row4_layout = QHBoxLayout()
        row4_layout.setContentsMargins(10, 5, 10, 5)

        info_label = QLabel()
        info_label.setFixedSize(110, 20)
        info_label.setAlignment(Qt.AlignRight | Qt.AlignTrailing | Qt.AlignVCenter)
        info_label.setText("Additional Info:")
        row4_layout.addWidget(info_label, alignment=Qt.Alignment(0))

        info_cb = QCheckBox('(Restart Required)')
        info_cb.setChecked(self.config['DEFAULT'].getboolean('additional_info'))
        row4_layout.addWidget(info_cb, alignment=Qt.Alignment(0))
        row4_layout.addStretch()

        row4.setLayout(row4_layout)
        main_layout.addWidget(row4, alignment=Qt.Alignment(0))

        # BUTTONS
        button_widget = QWidget(flags=self.flags)
        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(10, 5, 10, 10)

        button_box = QDialogButtonBox()
        button_box.setOrientation(Qt.Horizontal)
        button_box.setStandardButtons(QDialogButtonBox.Cancel | QDialogButtonBox.Ok)
        button_layout.addStretch()
        button_layout.addWidget(button_box, alignment=Qt.Alignment(0))
        button_layout.addStretch()
        button_widget.setLayout(button_layout)
        main_layout.addWidget(button_widget, alignment=Qt.Alignment(0))

        def update_config():
            self.config['DEFAULT']['replays_folder'] = replays.text()
            self.config['DEFAULT']['api_key'] = api_key.text()
            self.config['DEFAULT']['region'] = \
                [region for region, index in regions.items() if index == region_picker.currentIndex()][0]
            self.config['DEFAULT']['additional_info'] = 'true' if info_cb.isChecked() else 'false'

        def flag_config_reload_needed():
            self.config_reload_needed = True

        button_box.accepted.connect(mw.accept)
        button_box.accepted.connect(update_config)
        button_box.accepted.connect(self.config.save)
        button_box.accepted.connect(flag_config_reload_needed)
        button_box.rejected.connect(mw.reject)

        mw.windowContent.setLayout(main_layout)
        mw.exec()

    def fill_tables(self, players):
        self.left_table.clearContents()
        self.right_table.clearContents()

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
                if x == 0 and player.clan_tag:
                    text = f'<span style="color:{player.clan_color}"> [{player.clan_tag}]</span>{player.row[x]}'
                    item = Label(text=text, size=10, bold=False)
                    item.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)
                    item.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Ignored)
                    item.setContentsMargins(3, 0, 3, 0)
                    if player.background:
                        b = player.background
                        rgba = f"{b.red()}, {b.green()}, {b.blue()}, {b.alpha()}"
                        item.setAutoFillBackground(True)
                        item.setStyleSheet("QLabel { background-color: rgba("+rgba+"); font-size: 13px; }")
                    else:
                        item.setStyleSheet("QLabel { font-size: 13px; }")
                    table.setCellWidget(y, x, item)
                    continue

                font = QFont("Segoe UI", size, QFont.Bold) if x else QFont("Segoe UI", size)
                item = QTableWidgetItem(player.row[x])
                item.setFont(font)
                if player.background:
                    item.setBackground(player.background)
                if player.colors[x]:
                    item.setForeground(player.colors[x])
                if x > 1:
                    item.setTextAlignment(Qt.AlignRight)
                table.setItem(y, x, item)
                x += 1

            if player.background:  # Set background for empty columns
                for x in range(len(player.row), self.left_table.columnCount()):
                    item = QTableWidgetItem('')
                    item.setBackground(player.background)
                    table.setItem(y, x, item)

    def closeEvent(self, event):
        # Save current window position and size
        self.config['DEFAULT']['windowx'] = str(self.mw.geometry().x())
        self.config['DEFAULT']['windowy'] = str(self.mw.geometry().y())
        self.config['DEFAULT']['windowh'] = str(self.mw.geometry().height())
        self.config['DEFAULT']['windoww'] = str(self.mw.geometry().width())
        self.config.save()
        super().closeEvent(event)

    def notify_update(self):
        reply = QMessageBox.question(self, 'Update', 'Do you want to update now?')

        if reply == QMessageBox.Yes:
            # event.accept()
            pass
        else:
            # event.ignore()
            pass


def resource_path(relative_path):
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath('.'), relative_path)


def create_gui():
    import sys
    app = QApplication(sys.argv)
    ui = MainWindow()
    styles.dark(app, resource_path('./assets/style.qss'))
    ui.mw = windows.ModernWindow(ui, resource_path('./assets/frameless.qss'))
    ui.mw.show()
    # app.setStyle('Fusion')
    return app, ui

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

from assets.qtmodern import windows
from PyQt5.QtWidgets import (QLabel, QWidget, QTableWidgetItem, QMainWindow,  QMessageBox, QComboBox,
                             QSizePolicy, QHBoxLayout, QVBoxLayout, QTextEdit, QPushButton)
from PyQt5.QtGui import QIcon, QFont, QPixmap, QDesktopServices, QMovie, QTextCursor
from PyQt5.QtCore import Qt, QUrl, QSize
from gui.stats_table import StatsTable
from gui.label import Label
from gui.team_stats import TeamStats
from gui.menubar import VerticalMenuBar
from gui.settings import SettingsMenu
from gui.about import AboutMenu
from utils.resource_path import resource_path


# noinspection PyArgumentList
class MainWindow(QMainWindow):
    def __init__(self, config, pa):
        self.pa = pa
        self.config = config
        self.flags = Qt.WindowFlags()
        super().__init__(flags=self.flags)

        self.layout = QHBoxLayout()

        self.stats_widget = QWidget(self)
        self.stats_layout = QVBoxLayout()

        self.settings_widget = SettingsMenu(self)
        self.about_widget = AboutMenu(self)

        self.menu_bar = VerticalMenuBar(self.centralWidget())

        self.setup_ui()

        self.status_icon, self.status_text = None, None
        self.update_status(1, 'Ready')

        self.create_table_labels()

        self.left_table, self.right_table = self.create_tables()
        # self.create_menubar()
        self.team_stats = TeamStats(self.stats_layout, pa)
        self.mw = None
        self.connect_signals()

    def setup_ui(self):
        # self.setStyleSheet('border-style: solid; border-width: 0.5px; border-color: red;')
        self.layout.setContentsMargins(10, 0, 0, 10)
        self.layout.setSpacing(0)
        self.stats_layout.setContentsMargins(0, 0, 0, 0)
        self.stats_layout.setSpacing(0)

        self.setMouseTracking(False)
        self.setTabletTracking(False)
        # self.set_size()
        self.setWindowTitle('PotatoAlert')

        icon = QIcon()
        icon.addPixmap(QPixmap(resource_path('./assets/potato.png')), QIcon.Normal, QIcon.Off)
        self.setWindowIcon(icon)

        # self.setStatusBar(QStatusBar())

        self.stats_widget.setLayout(self.stats_layout)
        self.settings_widget.setVisible(False)
        self.about_widget.setVisible(False)

        self.setCentralWidget(QWidget(self))
        self.centralWidget().setLayout(self.layout)

        self.layout.addWidget(self.menu_bar)
        self.layout.addWidget(self.stats_widget)
        self.layout.addWidget(self.settings_widget)
        self.layout.addWidget(self.about_widget)

    def set_size(self):
        self.mw.move(self.config.getint('DEFAULT', 'windowx'), self.config.getint('DEFAULT', 'windowy'))
        self.mw.resize(self.config.getint('DEFAULT', 'windoww'), self.config.getint('DEFAULT', 'windowh'))

    def create_tables(self):
        table_widget = QWidget(flags=self.flags)
        table_layout = QHBoxLayout()
        table_layout.setContentsMargins(10, 0, 10, 0)
        table_layout.setSpacing(10)
        t1 = StatsTable()
        t2 = StatsTable()
        table_layout.addWidget(t1, alignment=Qt.Alignment(0))
        table_layout.addWidget(t2, alignment=Qt.Alignment(0))
        table_widget.setLayout(table_layout)
        self.stats_layout.addWidget(table_widget, alignment=Qt.Alignment(0))
        t1.doubleClicked.connect(t1.print_click)
        t2.doubleClicked.connect(t2.print_click)
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

        # TEMPORARY FIX FOR WGS BULLSHIT
        mode_label = QLabel('Mode:')
        right_layout.addWidget(mode_label, alignment=Qt.Alignment(0))
        self.mode_picker = QComboBox()
        self.mode_picker.addItems(['pvp', 'clan', 'ranked', 'cooperative', 'pve'])
        self.mode_picker.setCurrentIndex(0)
        self.mode_picker.setFixedSize(70, 20)
        right_layout.addWidget(self.mode_picker, alignment=Qt.Alignment(0))
        self.mode_picker.currentTextChanged.connect(self.pa.run)
        def set_mode():
            self.pa.mode = self.mode_picker.currentText()
        self.mode_picker.currentTextChanged.connect(set_mode)
        # TEMPORARY FIX FOR WGS BULLSHIT

        label_layout.addWidget(left_widget, alignment=Qt.Alignment(0))
        label_layout.addWidget(right_widget, alignment=Qt.Alignment(0))
        self.stats_layout.addWidget(label_widget, alignment=Qt.Alignment(0))

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

    def fill_tables(self):
        players = self.pa.players
        self.left_table.clearContents()
        self.right_table.clearContents()
        self.left_table.players = [p for p in players if p.team == 0 or p.team == 1]
        self.right_table.players = [p for p in players if p.team == 2]

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
                size = 13 if x < 2 else 16
                if x == 0 and player.clan_tag:
                    text = f'<span style="color:{player.clan_color};"> [{player.clan_tag}]</span>{player.row[x]}'
                    item = Label(text=text, size=10, bold=False)
                    item.setTextFormat(Qt.RichText)
                    item.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)
                    item.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Ignored)
                    item.setContentsMargins(3, 0, 3, 0)
                    if player.background:
                        b = player.background
                        rgba = f"{b.red()}, {b.green()}, {b.blue()}, {b.alpha()}"
                        item.setAutoFillBackground(True)
                        item.setStyleSheet(
                            f'background-color: rgba({rgba});'
                            f'font-size: {size}px;'
                            f'font-family: Segoe UI;'
                        )
                    else:
                        item.setStyleSheet(f'font-size: {size}px; font-family: Segoe UI;')
                    table.setCellWidget(y, x, item)
                    continue

                font = QFont('Segoe UI', size, QFont.Bold) if x else QFont('Segoe UI', size)
                font.setPixelSize(size)
                item = QTableWidgetItem(player.row[x])
                item.setFont(font)
                item.setTextAlignment(Qt.AlignVCenter)
                if player.background:
                    item.setBackground(player.background)
                if player.colors[x]:
                    item.setForeground(player.colors[x])
                if x > 1:
                    item.setTextAlignment(Qt.AlignVCenter | Qt.AlignRight)
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
        d = windows.ModernDialog(resource_path('./assets/frameless.qss'), parent=self, hide_window_buttons=True)
        d.setWindowTitle('Update Available')

        box = QMessageBox(d)
        box.setIcon(QMessageBox.Question)
        box.setText('There is a new version available.\nWould you like to update now?')
        box.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
        box.setEscapeButton(QMessageBox.No)
        box.setAttribute(Qt.WA_TranslucentBackground)
        box.setWindowFlags(Qt.Window | Qt.FramelessWindowHint | Qt.WindowSystemMenuHint)

        layout = QHBoxLayout()
        layout.addWidget(box, alignment=Qt.Alignment(0))
        d.windowContent.setLayout(layout)
        d.show()

        if box.exec_() == QMessageBox.Yes:
            d.hide()
            return True
        else:
            d.hide()
            return False

    def show_changelog(self, version: str, text: str):
        d = windows.ModernDialog(resource_path('./assets/frameless.qss'), parent=self, hide_window_buttons=True)
        d.setMinimumWidth(550)

        main_layout = QVBoxLayout()
        main_layout.setSpacing(10)
        d.setWindowTitle('Changelog')

        about_widget = QWidget(flags=self.flags)
        row_layout = QHBoxLayout()
        row_layout.setSpacing(10)
        row_layout.setContentsMargins(0, 0, 0, 0)

        pix = QPixmap(resource_path('./assets/potato.png'))
        pix = pix.scaled(70, 70)
        img = QLabel()
        img.setPixmap(pix)
        row_layout.addWidget(img, alignment=Qt.Alignment(0))

        changelog_widget = QWidget(flags=self.flags)
        changelog_layout = QVBoxLayout()
        changelog_layout.setSpacing(10)
        changelog_layout.setContentsMargins(0, 0, 0, 0)

        font = QFont()
        font.setPointSize(12)
        text1 = QLabel(f'New in version {version}')
        text1.setFont(font)
        changelog_layout.addWidget(text1, alignment=Qt.Alignment(0))

        changes = QTextEdit()
        changes.insertPlainText(text)
        changes.setReadOnly(True)
        changes.setFixedHeight(100)
        changes.moveCursor(QTextCursor.Start)
        changelog_layout.addWidget(changes, alignment=Qt.Alignment(0))

        changelog_widget.setLayout(changelog_layout)
        row_layout.addWidget(changelog_widget, alignment=Qt.Alignment(0))

        about_widget.setLayout(row_layout)
        main_layout.addWidget(about_widget, alignment=Qt.Alignment(0))

        ok_btn = QPushButton('OK')
        ok_btn.setDefault(False)
        ok_btn.setAutoDefault(False)
        ok_btn.setFixedWidth(100)
        ok_btn.clicked.connect(d.accept)
        main_layout.addWidget(ok_btn, alignment=Qt.AlignCenter)

        d.windowContent.setLayout(main_layout)
        d.exec()

    def switch_tab(self, new: int):
        old = self.menu_bar.btn_group.checkedId()
        tabs = {
            0: self.stats_widget,
            1: self.settings_widget,
            4: self.about_widget
        }
        if not old == new:
            if self.menu_bar.btn_group.buttons()[old].isChecked():
                self.menu_bar.btn_group.buttons()[old].setChecked(False)
            if not self.menu_bar.btn_group.buttons()[new].isChecked():
                self.menu_bar.btn_group.buttons()[new].setChecked(True)

        tabs[new].setVisible(True)
        [tab.setVisible(False) for _id, tab in tabs.items() if _id != new]

    def connect_signals(self):
        self.pa.signals.status.connect(self.update_status)
        self.pa.signals.players.connect(self.fill_tables)
        self.pa.signals.averages.connect(self.team_stats.update_avg)
        self.pa.signals.servers.connect(self.team_stats.update_servers)
        self.pa.signals.clans.connect(self.team_stats.update_clans)

        def button_actions(btn):
            btn_id = self.menu_bar.btn_group.id(btn)
            if btn_id == 2:
                QDesktopServices.openUrl(QUrl('https://github.com/razaqq/PotatoAlert'))
            elif btn_id == 3:
                QDesktopServices.openUrl(QUrl.fromLocalFile(self.config.config_path))
            else:
                self.switch_tab(btn_id)

        self.menu_bar.btn_group.buttonClicked.connect(button_actions)

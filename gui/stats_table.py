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

from PyQt5.QtWidgets import QTableWidget, QHeaderView, QTableWidgetItem, QAbstractItemView
from PyQt5.QtCore import Qt, QUrl
from PyQt5.QtGui import QFont, QDesktopServices


class StatsTable(QTableWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.init()
        self.init_headers()
        self.players = []

    def init(self):
        self.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.setSelectionMode(QAbstractItemView.NoSelection)
        self.setFocusPolicy(Qt.NoFocus)
        self.setAlternatingRowColors(False)
        self.setMouseTracking(True)
        self.setRowCount(12)
        self.setColumnCount(8)
        self.setSortingEnabled(False)
        self.setContentsMargins(0, 0, 0, 0)
        self.setCursor(Qt.PointingHandCursor)

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
        self.setCursor(Qt.PointingHandCursor)

    def print_click(self, a):
        try:
            p = self.players[a.row()]
            if not p.row[0]:
                return
            region = f'{p.region}.' if p.region != 'eu' else ''
            url = QUrl(f'https://{region}wows-numbers.com/player/{p.account_id},{p.row[0]}/')
            QDesktopServices.openUrl(url)
        except IndexError:
            pass
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

from PyQt5.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QPushButton, QButtonGroup
from PyQt5.QtCore import Qt, QSize
from PyQt5.QtGui import QIcon
from gui.custom_widget import CustomWidget
from utils.resource_path import resource_path


class MenuButton(QPushButton):
    def __init__(self, parent, icon_path, name):
        super().__init__(parent)
        self.name = name
        width = parent.width() - 2 * parent.layout().spacing()
        icon = QIcon(resource_path(icon_path))
        self.setIcon(icon)
        self.setIconSize(QSize(width, width))
        self.setCursor(Qt.PointingHandCursor)
        self.setCheckable(True)
        self.setFlat(True)
        self.setMouseTracking(True)

    def mouseMoveEvent(self, *args, **kwargs):
        pass


class MenuEntry(CustomWidget):
    def __init__(self, parent, icon_path, name):
        super().__init__(parent)
        self.setFixedWidth(parent.width())
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(5)
        self.setLayout(layout)
        self.btn = MenuButton(self, icon_path, name)
        layout.addWidget(self.btn)


class VerticalMenuBar(CustomWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName('VerticalMenuBar')
        self.layout = QVBoxLayout()
        self.setFixedWidth(30)
        self.btn_group = QButtonGroup(self)
        self.table_entry, self.settings_entry, self.about_entry, self.github_entry, self.logs_entry = [None] * 5
        self.setup_ui()

    def setup_ui(self):
        self.setLayout(self.layout)
        self.layout.setContentsMargins(0, 10, 0, 10)
        self.layout.setSpacing(0)

        self.table_entry = MenuEntry(self, 'assets/menuicons/table.svg', 'table')
        self.table_entry.btn.setChecked(True)
        self.layout.addWidget(self.table_entry, False, Qt.AlignTop | Qt.AlignHCenter)

        self.settings_entry = MenuEntry(self, 'assets/menuicons/settings.svg', 'settings')
        self.layout.addWidget(self.settings_entry, False, Qt.AlignTop | Qt.AlignHCenter)

        self.layout.addStretch()

        self.github_entry = MenuEntry(self, 'assets/menuicons/github.svg', 'github')
        self.github_entry.btn.setCheckable(False)
        self.layout.addWidget(self.github_entry, False, Qt.AlignTop | Qt.AlignHCenter)

        self.logs_entry = MenuEntry(self, 'assets/menuicons/log.svg', 'logs')
        self.logs_entry.btn.setCheckable(False)
        self.layout.addWidget(self.logs_entry, False, Qt.AlignTop | Qt.AlignHCenter)

        self.about_entry = MenuEntry(self, 'assets/menuicons/about.svg', 'about')
        self.layout.addWidget(self.about_entry, False, Qt.AlignTop | Qt.AlignHCenter)

        self.btn_group.setExclusive(True)
        self.btn_group.addButton(self.table_entry.btn)
        self.btn_group.addButton(self.settings_entry.btn)
        self.btn_group.addButton(self.about_entry.btn)
        self.btn_group.addButton(self.github_entry.btn)
        self.btn_group.addButton(self.logs_entry.btn)


if __name__ == '__main__':
    from PyQt5.QtWidgets import QApplication, QMainWindow, QTableWidget, QTableWidgetItem
    from assets.qtmodern.styles import dark
    from assets.qtmodern.windows import ModernWindow
    from utils.resource_path import resource_path

    app = QApplication([])
    dark(app, resource_path('assets/style.qss'))
    m = QMainWindow()
    mw = ModernWindow(m, resource_path('assets/frameless.qss'))
    mw.resize(1500, 600)
    mw_layout = QHBoxLayout()
    mw_layout.setContentsMargins(0, 0, 0, 0)
    mw_layout.setSpacing(0)

    # m.setStyleSheet('border-style: solid; border-width: 0.5px; border-color: blue;')

    central_widget = QWidget()
    central_widget.setLayout(mw_layout)

    m.setCentralWidget(central_widget)

    menubar = VerticalMenuBar(central_widget)
    print(menubar)
    mw_layout.addWidget(menubar, alignment=Qt.AlignLeft)

    t = QTableWidget()
    t.setRowCount(10)
    t.setColumnCount(10)
    for i in range(10):
        for j in range(10):
            t.setItem(i, j, QTableWidgetItem('test'))
    mw_layout.addWidget(t)
    menubar.table_widget = t

    mw.show()
    app.exec()

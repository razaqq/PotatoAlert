from PyQt5.QtWidgets import QApplication, QLabel, QTableWidget, QWidget, QTableWidgetItem, QAbstractItemView,\
     QSizePolicy, QMainWindow, QHeaderView
from PyQt5.QtGui import QIcon, QFont, QPixmap
from PyQt5.QtCore import QRect, Qt, QSize
from textcolors import Orange, Purple, Cyan


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
        # self.create_tables()
        self.left_table = Table(self.central_widget, 10, 30)
        self.right_table = Table(self.central_widget, 755, 30)
        self.create_table_labels()

    def init(self):
        self.setObjectName("MainWindow")
        self.setMouseTracking(False)
        self.setTabletTracking(False)
        self.set_size()
        self.setWindowTitle("PotatoAlert")
        icon = QIcon()
        icon.addPixmap(QPixmap("potato.png"), QIcon.Normal, QIcon.Off)
        self.setWindowIcon(icon)
        self.setAutoFillBackground(False)

        self.central_widget = QWidget(self, flags=self.flags)
        self.setCentralWidget(self.central_widget)
        # QMetaObject.connectSlotsByName(self)

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
        right_label = Label(self.central_widget, 1020, 0, 'Enemy Team')

    def fill_tables(self, players):
        tables = {1: 1, 2: 0}
        table = None
        y = 0
        for player in players:
            if player.relation == 0:
                table = self.left_table
                y = 0
            if player.relation == 1:
                table = self.left_table
                y = tables[1]
                tables[1] += 1
            if player.relation == 2:
                table = self.right_table
                y = tables[2]
                tables[2] += 1

            player_name = f'[{player.clan}] {player.name}' if player.clan else player.name
            table.setItem(y, 0, QTableWidgetItem(player_name))
            table.setItem(y, 1, QTableWidgetItem('Montana'))

            if not player.hidden_profile:
                battles = player.stats['battles']
                winrate = round(player.stats['wins'] / battles * 100, 1)
                avg_dmg = int(player.stats['damage_dealt'] / battles)

                item = QTableWidgetItem(str(battles))
                item.setForeground(Orange())
                table.setItem(y, 2, item)

                table.setItem(y, 3, QTableWidgetItem(str(winrate)))
                table.setItem(y, 4, QTableWidgetItem(str(avg_dmg)))
            else:
                table.setItem(y, 0, QTableWidgetItem(player_name))
                table.setItem(y, 2, QTableWidgetItem('Hidden'))
                table.setItem(y, 3, QTableWidgetItem('Profile'))
                table.setItem(y, 4, QTableWidgetItem(':('))


def create_gui():
    import sys
    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    ui = MainWindow()
    return app, ui


if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    ui = MainWindow()
    ui.show()
    sys.exit(app.exec_())

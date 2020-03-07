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

from PyQt5.QtWidgets import QWidget, QHBoxLayout
from PyQt5.QtCore import Qt
from gui.label import Label


class TeamStats:
    def __init__(self, main_layout, pa):
        self.pa = pa
        self.flags = Qt.WindowFlags()
        widget = QWidget(flags=self.flags)
        layout = QHBoxLayout()
        layout.setContentsMargins(10, 0, 10, 0)
        layout.setSpacing(10)
        widget.setLayout(layout)

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
        self.update_avg()
        self.update_servers()

    def update_avg(self):
        team1, team2 = self.pa.avg
        self.t1_wr.setText(f'{team1.winrate}%')
        self.t1_dmg.setText(f'{team1.avg_dmg}')
        self.t2_wr.setText(f'{team2.winrate}%')
        self.t2_dmg.setText(f'{team2.avg_dmg}')

        self.t1_wr.setStyleSheet(f"color: rgb({team1.winrate_c[0]}, {team1.winrate_c[1]}, {team1.winrate_c[2]})")
        self.t1_dmg.setStyleSheet(f"color: rgb({team1.avg_dmg_c[0]}, {team1.avg_dmg_c[1]}, {team1.avg_dmg_c[2]})")
        self.t2_wr.setStyleSheet(f"color: rgb({team2.winrate_c[0]}, {team2.winrate_c[1]}, {team2.winrate_c[2]})")
        self.t2_dmg.setStyleSheet(f"color: rgb({team2.avg_dmg_c[0]}, {team2.avg_dmg_c[1]}, {team2.avg_dmg_c[2]})")

    def update_servers(self):
        t1_server, t2_server = self.pa.servers
        if t1_server and t2_server:
            self.t1_server.show(), self.t2_server.show()
            self.t1_server_label.show(), self.t2_server_label.show()
            self.t1_server.setText(f'{t1_server}')
            self.t2_server.setText(f'{t2_server}')
        else:
            self.t1_server.hide(), self.t2_server.hide()
            self.t1_server_label.hide(), self.t2_server_label.hide()

    def update_clans(self):
        c1, c2 = self.pa.clans
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

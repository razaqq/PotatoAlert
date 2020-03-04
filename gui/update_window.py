from PyQt5.QtWidgets import QMainWindow, QProgressBar, QWidget, QLabel, QStatusBar, QVBoxLayout, QHBoxLayout
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QIcon, QPixmap, QFont
from utils.config import Config
from utils.resource_path import resource_path


class UpdateWindow(QMainWindow):
    def __init__(self):
        self.flags = Qt.WindowFlags()
        super().__init__(flags=self.flags)
        self.config = Config()
        self.central_widget = QWidget(self, flags=self.flags)
        self.setCentralWidget(self.central_widget)
        self.layout = QVBoxLayout()
        self.central_widget.setLayout(self.layout)
        self.progress_bar = QProgressBar(self)
        self.speed = QLabel()
        self.progress_mb = QLabel()
        self.init()

    def init(self):
        self.setWindowTitle('PotatoAlert Updater')
        icon = QIcon()
        icon.addPixmap(QPixmap(resource_path('./assets/potato.png')), QIcon.Normal, QIcon.Off)
        self.setWindowIcon(icon)
        self.progress_bar.setValue(0)
        self.progress_bar.setBaseSize(150, 20)
        upd = QLabel('Updating, please wait...')
        upd.setFont(QFont('Segoe UI', 12, QFont.Bold))
        upd.setAlignment(Qt.AlignHCenter | Qt.AlignTop)
        self.layout.addWidget(upd, alignment=Qt.Alignment(0))
        self.layout.addWidget(self.progress_bar, alignment=Qt.Alignment(0))
        pgr_widget = QWidget(flags=self.flags)
        pgr_layout = QHBoxLayout()
        pgr_layout.addStretch()
        pgr_layout.addWidget(self.progress_mb, alignment=Qt.Alignment(0))
        pgr_layout.addStretch()
        pgr_layout.addWidget(self.speed, alignment=Qt.Alignment(0))
        pgr_layout.addStretch()
        pgr_widget.setLayout(pgr_layout)
        pgr_layout.setSpacing(0)
        pgr_layout.setContentsMargins(0, 0, 0, 0)
        self.layout.addWidget(pgr_widget, alignment=Qt.Alignment(0))

    async def update_progress(self, func=None):
        async for percent, mb_done, mb_total, rate in func():
            self.progress_bar.setValue(percent)
            self.progress_mb.setText(f'{mb_done}/{mb_total} MB')
            self.speed.setText(f'{rate} MB/s')

    def set_size(self):
        self.mw.resize(300, 100)
        self.move(self.config.getint('DEFAULT', 'windowx'), self.config.getint('DEFAULT', 'windowy'))

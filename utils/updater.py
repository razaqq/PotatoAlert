import os
import sys
from aiohttp import ClientSession
import aiofiles
from PyQt5.QtWidgets import QMainWindow, QProgressBar, QWidget, QVBoxLayout, QStatusBar, QLabel, QApplication
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QIcon, QPixmap
from assets.qtmodern import styles, windows
from utils.config import Config
from version import __version__

file_name = os.path.basename(sys.executable)
current_path = os.path.dirname(sys.executable)

# Check if old version around and delete it
if os.path.exists(os.path.join(current_path, f'{file_name}_old')):
    os.remove(os.path.join(current_path, f'{file_name}_old'))


def resource_path(relative_path):
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath('.'), relative_path)


async def check_update():
    # Check if update needed
    update_needed = False
    if getattr(sys, 'frozen', False):
        try:
            url = 'https://raw.githubusercontent.com/razaqq/PotatoAlert/master/version.py'
            async with ClientSession() as s:
                async with s.get(url) as resp:
                    new_version = await resp.text()
                    new_version = new_version.split("'")[1]
            if __version__ < new_version:
                update_needed = True
            return update_needed
        except ConnectionError:
            return False
    else:
        return False  # TODO notify of update if someone is not using frozen build


async def update():
    try:
        os.rename(os.path.join(current_path, file_name), os.path.join(current_path, f'{file_name}_old'))
        url = "https://github.com/razaqq/PotatoAlert/releases/latest/download/potatoalert_x64.exe"
        async with ClientSession() as s:
            async with s.get(url) as resp:
                if resp.status == 200:
                    size = int(resp.headers.get('content-length', '0')) or None
                    downloaded = 0

                    f = await aiofiles.open(os.path.join(current_path, file_name), mode='wb')
                    async for chunk in resp.content.iter_chunked(512):
                        await f.write(chunk)
                        downloaded += len(chunk)
                        yield downloaded / size * 100
                    await f.close()
    except ConnectionError:
        os.rename(os.path.join(current_path, f'{file_name}_old'), os.path.join(current_path, file_name))
    finally:
        os.execl(os.path.join(current_path, file_name), os.path.join(current_path, file_name))


class UpdateWindow(QMainWindow):
    def __init__(self):
        self.flags = Qt.WindowFlags()
        super().__init__(flags=self.flags)
        self.central_widget = QWidget(self, flags=self.flags)
        self.setCentralWidget(self.central_widget)
        self.layout = QVBoxLayout()
        self.central_widget.setLayout(self.layout)
        self.progress = QProgressBar(self)
        self.init()

    def init(self):
        c = Config()
        self.setWindowTitle('PotatoAlert Updater')
        icon = QIcon()
        icon.addPixmap(QPixmap(resource_path('./assets/potato.png')), QIcon.Normal, QIcon.Off)
        self.setWindowIcon(icon)
        self.setStatusBar(QStatusBar())
        self.resize(300, 100)
        self.move(c.getint('DEFAULT', 'windowx'), c.getint('DEFAULT', 'windowy'))
        self.progress.setValue(0)
        self.progress.setBaseSize(150, 20)
        upd = QLabel('Updating, please wait...')
        upd.setFont(QFont('Segoe UI', 12, QFont.Bold))
        upd.setAlignment(Qt.AlignHCenter | Qt.AlignTop)
        self.layout.addWidget(upd, alignment=Qt.Alignment(0))
        self.layout.addWidget(self.progress, alignment=Qt.Alignment(0))

    async def update_progress(self, func=None):
        async for prog in func():
            self.progress.setValue(prog)


def create_gui():
    import sys
    app = QApplication(sys.argv)
    ui = UpdateWindow()
    styles.dark(app, resource_path('./assets/style.qss'))
    ui.mw = windows.ModernWindow(ui, resource_path('./assets/frameless.qss'))
    ui.mw.show()
    return app, ui

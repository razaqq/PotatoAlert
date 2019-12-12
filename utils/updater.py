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
import time
from aiohttp import ClientSession
import aiofiles
from pkg_resources import parse_version
from PyQt5.QtWidgets import QMainWindow, QProgressBar, QWidget, QVBoxLayout, QStatusBar, QLabel, QApplication,\
                            QHBoxLayout
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QIcon, QPixmap
from assets.qtmodern import styles, windows
from utils.config import Config
from aiohttp.client_exceptions import ClientResponseError, ClientError, ClientConnectionError
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
            if parse_version(__version__) < parse_version(new_version):
                update_needed = True
            return update_needed
        except (ClientResponseError, ClientError, ClientConnectionError, ConnectionError):
            return False
    else:
        return False  # TODO notify of update if someone is not using frozen build


async def update():
    try:
        try:
            os.rename(os.path.join(current_path, file_name), os.path.join(current_path, f'{file_name}_old'))
        except Exception as e:
            print(e)
        url = "https://github.com/razaqq/PotatoAlert/releases/latest/download/potatoalert_x64.exe"
        async with ClientSession() as s:
            async with s.get(url) as resp:
                if resp.status == 200:
                    size = int(resp.headers.get('content-length', '0')) or None
                    downloaded = 0

                    f = await aiofiles.open(os.path.join(current_path, file_name), mode='wb')
                    start_time = time.time()
                    async for chunk in resp.content.iter_any():
                        await f.write(chunk)
                        downloaded += len(chunk)
                        percent = downloaded / size * 100
                        done = round(downloaded / 1E6, 1)
                        total = round(size / 1E6, 1)
                        try:
                            rate = round(downloaded / 1E6 / (time.time() - start_time), 2)
                        except ZeroDivisionError:  # If your internet is too fast, first tick might be zero. 16ms pres.
                            rate = 0
                        yield percent, done, total, rate
                    await f.close()
    except (ClientResponseError, ClientError, ClientConnectionError, ConnectionError) as e:
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
        self.progress_bar = QProgressBar(self)
        self.speed = QLabel()
        self.progress_mb = QLabel()
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


def create_gui():
    import sys
    app = QApplication(sys.argv)
    ui = UpdateWindow()
    styles.dark(app, resource_path('./assets/style.qss'))
    mw = windows.ModernWindow(ui, resource_path('./assets/frameless.qss'), hide_window_buttons=True)
    mw.show()
    return app, ui

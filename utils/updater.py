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
import ctypes
from aiohttp import ClientSession
import aiofiles
from PyQt5.QtWidgets import QApplication
from assets.qtmodern import styles, windows
from gui.update_window import UpdateWindow
from aiohttp.client_exceptions import ClientResponseError, ClientError, ClientConnectionError
from utils.resource_path import resource_path
from version import __version__


def full_path(filename: str):
    return os.path.join(current_path, filename)


current_name = os.path.basename(sys.executable)
temp_name = current_name + '_temp'
old_name = current_name + '_old'
current_path = os.path.dirname(sys.executable)

# Check if old version around and delete it
if os.path.exists(full_path(old_name)):
    os.remove(full_path(old_name))


def parse_version(v):
    return tuple(map(int, (v.split("."))))


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
        url = 'https://github.com/razaqq/PotatoAlert/releases/latest/download/potatoalert_x64.exe'
        async with ClientSession() as s:
            async with s.get(url) as resp:
                if resp.status == 200:
                    size = int(resp.headers.get('content-length', '0')) or None
                    downloaded = 0

                    f = await aiofiles.open(full_path(temp_name), mode='wb')
                    start_time = time.time()
                    async for chunk in resp.content.iter_any():
                        await f.write(chunk)
                        downloaded += len(chunk)
                        percent = downloaded / size * 100
                        done = round(downloaded / 1E6, 1)
                        total = round(size / 1E6, 1)
                        try:
                            rate = round(downloaded / 1E6 / (time.time() - start_time), 2)
                        except ZeroDivisionError:  # If your internet is too fast, first tick might be zero. 16ms prec.
                            rate = 0
                        yield percent, done, total, rate
                    await f.close()
    except Exception as e:  # any exception
        if os.path.exists(full_path(temp_name)):  # remove failed download
            os.remove(full_path(temp_name))
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, '', None, 1)  # restart current binary
    finally:
        os.rename(full_path(current_name), full_path(old_name))  # move current to old
        os.rename(full_path(temp_name), full_path(current_name))  # move new to current
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, '--changelog', None, 1)  # restart


def queue_update():
    if not ctypes.windll.shell32.IsUserAnAdmin():  # need admin for some folders
        ctypes.windll.shell32.ShellExecuteW(None, 'runas', sys.executable, '--update', None, 1)  # restart
        sys.exit(0)
    else:
        os.execl(os.path.join(current_path, current_name), os.path.join(current_path, current_name), '--update')


async def get_changelog():
    try:
        url = 'https://api.github.com/repos/razaqq/PotatoAlert/releases/latest'
        async with ClientSession() as s:
            async with s.get(url) as resp:
                res = await resp.json()
                return res['body']
    except (KeyError, TypeError, IndexError, ClientResponseError, ClientError, ClientConnectionError, ConnectionError):
        return ''


def create_gui():
    import sys
    app = QApplication(sys.argv)
    ui = UpdateWindow()
    styles.dark(app)
    ui.mw = windows.ModernWindow(ui)
    ui.mw.title_bar.btn_close.setVisible(False)
    ui.mw.title_bar.btn_maximize.setVisible(False)
    ui.mw.title_bar.btn_minimize.setVisible(False)
    ui.mw.title_bar.btn_restore.setVisible(False)
    ui.mw.show()
    app.processEvents()
    ui.set_size()
    return app, ui

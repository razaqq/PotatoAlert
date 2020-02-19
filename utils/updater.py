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
from PyQt5.QtWidgets import QApplication
from assets.qtmodern import styles, windows
from gui.update_window import UpdateWindow
from aiohttp.client_exceptions import ClientResponseError, ClientError, ClientConnectionError
from utils.resource_path import resource_path
from version import __version__

file_name = os.path.basename(sys.executable)
current_path = os.path.dirname(sys.executable)

# Check if old version around and delete it
if os.path.exists(os.path.join(current_path, f'{file_name}_old')):
    os.remove(os.path.join(current_path, f'{file_name}_old'))


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
        try:
            os.rename(os.path.join(current_path, file_name), os.path.join(current_path, f'{file_name}_old'))
        except Exception as e:
            print(e)
        url = 'https://github.com/razaqq/PotatoAlert/releases/latest/download/potatoalert_x64.exe'
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
                        except ZeroDivisionError:  # If your internet is too fast, first tick might be zero. 16ms prec.
                            rate = 0
                        yield percent, done, total, rate
                    await f.close()
    except (ClientResponseError, ClientError, ClientConnectionError, ConnectionError) as e:
        os.rename(os.path.join(current_path, f'{file_name}_old'), os.path.join(current_path, file_name))
    finally:
        os.execl(os.path.join(current_path, file_name), os.path.join(current_path, file_name), '--changelog')


def queue_update():
    os.execl(os.path.join(current_path, file_name), os.path.join(current_path, file_name), '--update')


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
    styles.dark(app, resource_path('./assets/style.qss'))
    mw = windows.ModernWindow(ui, resource_path('./assets/frameless.qss'), hide_window_buttons=True)
    mw.show()
    return app, ui

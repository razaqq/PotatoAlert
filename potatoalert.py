#!/usr/bin/env python3

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
import json
import asyncio
import logging
import argparse
import traceback
from json import JSONDecodeError
from hashlib import md5
from aiohttp.client_exceptions import ClientResponseError, ClientError, ClientConnectionError
from utils.config import Config
from asyncqt import QEventLoop
from utils.central_api import CentralApi
from utils.custom_api import CustomApi
from utils.api import ApiWrapper
from utils.dcs import Team
from utils.analytics import run_ga
from utils import updater
from gui.main_window import MainWindow
from version import __version__
from PyQt5.QtCore import pyqtSignal, QObject, QFileSystemWatcher
from PyQt5.QtWidgets import QApplication
from assets.qtmodern import styles, windows
from utils.resource_path import resource_path


class Signals(QObject):
    status = pyqtSignal(int, str)
    players = pyqtSignal()
    averages = pyqtSignal()
    servers = pyqtSignal()
    clans = pyqtSignal()


class PotatoAlert:
    def __init__(self, config: Config):
        self.config = config

        self.central_api = CentralApi(config, self)
        self.custom_api = CustomApi(config, self)

        # variables watched by gui
        self.signals = Signals()
        self.team1 = []
        self.team2 = []
        self.arena_info = None
        self.servers = (None, None)
        self.avg = (Team(), Team())
        self.clans = (None, None)
        self.config_reload_needed = False
        self.last_hash = ''

        self.arena_info_file, self.invalid_api_key, self.api = [None] * 3
        self.setup_logger()
        asyncio.get_running_loop().run_until_complete(self.reload_config())
        self._watcher = QFileSystemWatcher()  # watch arena info file for modification
        self._watcher.addPath(self.config['DEFAULT']['replays_folder'])
        self._watcher.directoryChanged.connect(self.run)

    def setup_logger(self):
        logging.basicConfig(level=logging.ERROR,
                            format='%(asctime)s - %(levelname)-5s:  %(message)s',
                            datefmt='%Y-%m-%d %H:%M:%S',
                            filename=os.path.join(self.config.config_path, 'Error.log'),
                            filemode='a')

    def set_config_reload_needed(self):
        self.config_reload_needed = True

    async def reload_config(self):
        self.arena_info_file = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        self.api = ApiWrapper(self.config['DEFAULT']['api_key'], self.config['DEFAULT']['region'])
        if not self.config['DEFAULT'].getboolean('use_central_api'):
            self.invalid_api_key = await self.custom_api.check_api_key()
        else:
            self.signals.status.emit(1, 'Ready')

    def run(self):
        try:
            t = asyncio.create_task(self._run())
            t.type_run = True  # hacky way for python3.7
        except asyncio.CancelledError:
            pass

    async def _run(self):
        for task in asyncio.all_tasks():  # cancel all other running tasks
            if not (task.done() or task == asyncio.current_task()) and (hasattr(task, 'type_run')):
                task.cancel()
        if self.config_reload_needed:
            await self.reload_config()
            self.config_reload_needed = False
        if os.path.exists(self.arena_info_file):
            # prevent duplicate run on same arena info file
            file_hash = md5(open(self.arena_info_file, 'rb').read()).hexdigest()
            if file_hash == self.last_hash:
                print('aborting run')
                return
            else:
                self.last_hash = file_hash

            try:
                if not self.config['DEFAULT'].getboolean('use_central_api') and self.invalid_api_key:
                    return self.signals.status.emit(3, 'Invalid API')
                self.arena_info = self.read_arena_info()
                if not self.arena_info:
                    return
                if self.config['DEFAULT'].getboolean('use_central_api'):
                    self.team1, self.team2, self.avg = await self.central_api.get_players(self.arena_info)
                else:
                    self.team1, self.team2, self.avg = await self.custom_api.get_players(self.arena_info)
                self.signals.players.emit()
                self.signals.averages.emit()
                self.signals.status.emit(1, 'Ready')
            except asyncio.CancelledError:
                self.signals.status.emit(1, 'Ready')  # do nothing
            except asyncio.TimeoutError:
                logging.exception('Connection with server timed out.')
                self.signals.status.emit(3, 'Connection Timeout')
            except ClientConnectionError:
                logging.exception('Check your internet connection!')
                self.signals.status.emit(3, 'Connection')
            except ConnectionRefusedError:
                logging.exception('Connection refused by remote host!')
                self.signals.status.emit(3, 'Central Server')
            except EOFError:
                logging.exception('Connection was unexpectedly closed by remote host!')
                self.signals.status.emit(3, 'EOFError')
            except (JSONDecodeError, TypeError, ValueError):
                logging.exception('Received invalid json response from server.')
                self.signals.status.emit(3, 'Response-Error')
            except (ClientError, ClientResponseError, Exception) as e:
                logging.exception(e)
                self.signals.status.emit(3, 'Check Logs')

    def read_arena_info(self) -> dict:
        arena_info = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        if not os.path.exists(arena_info):
            return {}
        with open(arena_info, 'r') as f:
            data = json.load(f)
            data['region'] = self.config['DEFAULT']['region']
            return data


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--update', dest='perform_update', action='store_true')
    parser.add_argument('--changelog', dest='show_changelog', action='store_true')
    args = parser.parse_args()

    try:
        loop = asyncio.get_event_loop()
        update_available = loop.run_until_complete(updater.check_update())

        if args.perform_update:
            app, gui = updater.create_gui()
            loop = QEventLoop(app)
            asyncio.set_event_loop(loop)
            loop.run_until_complete(asyncio.sleep(0.1))
            loop.run_until_complete(gui.update_progress(updater.update))
            sys.exit(0)

        config = Config()

        app = QApplication(sys.argv)

        loop = QEventLoop(app)
        asyncio.set_event_loop(loop)
        loop.run_until_complete(asyncio.sleep(0.1))

        pa = PotatoAlert(config)
        ui = MainWindow(config, pa)
        styles.dark(app)
        ui.mw = windows.ModernWindow(ui)
        with open(resource_path('assets/style.qss')) as s:
            app.setStyleSheet(app.styleSheet() + s.read())
        ui.mw.show()
        app.processEvents()
        ui.set_size()

        if update_available:
            perform_update = ui.notify_update()
            if perform_update:
                updater.queue_update()
        if args.show_changelog:
            changelog = loop.run_until_complete(updater.get_changelog())
            ui.show_changelog(__version__, changelog)

        if pa.config['DEFAULT'].getboolean('ga'):
            loop.run_until_complete(run_ga())

        pa.run()
        with loop:
            rc = loop.run_forever()
            for task in asyncio.all_tasks():
                task.cancel()
            del pa.api, pa, app, ui
            sys.exit(rc)
    except Exception as e:
        logging.exception(e)
        traceback.print_exc(e)

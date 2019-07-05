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
from requests import get, exceptions
from config import Config
from dataclasses import dataclass
from gui import create_gui
from asyncqt import QEventLoop
from apierrors import InvalidApplicationIdError
from version import __version__


@dataclass()
class Player:
    ship_id: str
    relation: int
    id: int
    name: str
    ship: str = None
    account_id: int = -1
    hidden_profile: bool = False
    stats: dict = None
    ship_stats: dict = None
    clan: str = None


class PotatoAlert:
    def __init__(self, ui):
        self.ui = ui
        self.config = Config()
        self.arena_info_file = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        self.client_version = ''
        self.steam_version = False
        self.region = ''
        self.current_players = []
        self.last_started = 0.0
        self.last_config_edit = float(os.stat(os.path.join(self.config.config_path, 'config.ini')).st_mtime)
        self.api = ApiWrapper(self.config['DEFAULT']['api_key'], self.config['DEFAULT']['region'])
        self.check_version()

    def check_version(self):
        try:
            url = 'https://raw.githubusercontent.com/razaqq/PotatoAlert/master/version.py'
            new_version = str(get(url).content.decode().split("'")[1])
            if __version__ != new_version:
                self.ui.notify_update(new_version)
        except exceptions.ConnectionError:
            pass

    async def run(self):
        while True:
            last_config_edit = float(os.stat(os.path.join(self.config.config_path, 'config.ini')).st_mtime)
            if last_config_edit > self.last_config_edit:  # TODO FIX THIS MESS
                self.config = Config()
                self.arena_info_file = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
                self.api = ApiWrapper(self.config['DEFAULT']['api_key'], self.config['DEFAULT']['region'])
                self.last_config_edit = last_config_edit
            if os.path.exists(self.arena_info_file):
                last_started = float(os.stat(
                    os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')).st_mtime)
                if last_started != self.last_started:
                    self.last_started = last_started
                    self.ui.set_status_bar('New game started, getting stats...')
                    await asyncio.sleep(0.05)
                    self.read_arena_info()

                    for player in self.current_players:
                        try:
                            player.account_id = self.api.get_account_info(player.name)['data'][0]['account_id']
                            player_s = self.api.get_player_stats(player.account_id)['data'][str(player.account_id)]
                            player.hidden_profile = player_s['hidden_profile']

                            # Get ship infos and replace type and nation with int for easier sorting
                            player.ship = self.api.get_ship_infos(player.ship_id)['data'][str(player.ship_id)]
                            if player.ship:  # sometimes we get None
                                ships = {
                                    'AirCarrier': 4,
                                    'Battleship': 3,
                                    'Cruiser': 2,
                                    'Destroyer': 1
                                }
                                player.ship['type'] = ships[player.ship['type']]

                                nations = {
                                    'usa': 10,
                                    'uk': 9,
                                    'commonwealth': 8,
                                    'france': 7,
                                    'germany': 6,
                                    'italy': 5,
                                    'japan': 4,
                                    'pan-asia': 3,
                                    'poland': 2,
                                    'russia': 1
                                }
                                player.ship['nation'] = nations.get(player.ship['nation'], 0)

                                # get detailed stats about ship if profile is not private
                                if not player.hidden_profile:
                                    player.ship_stats = self.api.get_ship_stats(
                                        player.account_id, player.ship_id)['data'][str(player.account_id)][0]['pvp']

                            # Check if player is in clan, if so get tag
                            clan_id = self.api.get_player_clan(player.account_id
                                                               )['data'][str(player.account_id)]['clan_id']
                            if clan_id:
                                player.clan = self.api.get_clan_details(clan_id)['data'][str(clan_id)]['tag']

                            # get general stats if profile is not private
                            if not player.hidden_profile:
                                player.stats = player_s['statistics']['pvp']
                        except InvalidApplicationIdError:
                            self.ui.set_status_bar('Invalid Application ID, please check your settings!')
                    self.ui.fill_tables(self.current_players)
                    self.ui.set_status_bar('Done.')
            self.ui.set_status_bar('Waiting for match start...')
            await asyncio.sleep(5)



    def read_arena_info(self):
        arena_info = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        if not os.path.exists(arena_info):
            return
        with open(arena_info, 'r') as f:
            data = json.load(f)
            self.client_version = data['clientVersionFromExe']

            players = []
            for p in data['vehicles']:
                players.append(Player(p['shipId'], p['relation'], p['id'], p['name']))
            self.current_players = players


class ApiWrapper:
    def __init__(self, api_key, region):
        self.api_key = api_key
        http_ending = region if region != 'na' else 'com'
        self.endpoint = f'https://api.worldofwarships.{http_ending}/wows/{{}}/{{}}/?'

    def get_result(self, method_block: str, method_name: str, params: dict) -> dict:
        try:
            params = {k: v for k, v in params.items() if v or isinstance(v, int)}
            url = self.endpoint.format(method_block, method_name)
            params['application_id'] = self.api_key
            res = get(url, params=params)
            res = res.json()
            if res['status'] == 'error':
                if res['error']['code'] == 407 and res['error']['message'] == 'INVALID_APPLICATION_ID':
                    raise InvalidApplicationIdError
            return res
        except exceptions.RequestException:
            print('Connection Error')

    def get_account_info(self, name) -> dict:
        param = {
            'search': name,
            'type': 'exact',
            'fields': 'account_id'
        }
        return self.get_result('account', 'list', param)

    def get_ship_stats(self, account_id: int, ship_id: int = 0) -> dict:
        param = {
            'account_id': account_id,
            'fields': 'pvp.battles,pvp.wins,pvp.damage_dealt',
            'ship_id': ship_id
        }
        return self.get_result('ships', 'stats', param)

    def get_player_stats(self, account_id: int) -> dict:
        param = {
            'account_id': account_id,
            'fields': 'hidden_profile,statistics.pvp.battles,statistics.pvp.losses,statistics.pvp.survived_battles,'
                      'statistics.pvp.wins,statistics.pvp.damage_dealt,statistics.pvp.frags,statistics.pvp.draws,'
                      'statistics.pvp.xp',
        }
        return self.get_result('account', 'info', param)

    def get_player_clan(self, account_id: int) -> dict:
        param = {
            'account_id': account_id
        }
        return self.get_result('clans', 'accountinfo', param)

    def get_clan_details(self, clan_id: int) -> dict:
        param = {
            'clan_id': clan_id,
            'fields': 'tag,members_count'
        }
        return self.get_result('clans', 'info', param)

    def get_ship_infos(self, ship_id: int) -> dict:
        param = {
            'ship_id': ship_id,
            'fields': 'name,tier,type,nation'
        }
        return self.get_result('encyclopedia', 'ships', param)


if __name__ == '__main__':
    app, ui = create_gui()
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)
    ui.show()
    loop.run_until_complete(asyncio.sleep(1))

    p = PotatoAlert(ui)
    loop.create_task(p.run())
    with loop:
        sys.exit(loop.run_forever())

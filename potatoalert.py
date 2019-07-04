#!/usr/bin/env python3

import os
import sys
import json
import asyncio
from requests import get
from configparser import ConfigParser
from dataclasses import dataclass
from gui import create_gui
from asyncqt import QEventLoop
from apierrors import InvalidApplicationIdError


@dataclass()
class Player:
    ship_id: str
    relation: int
    id: int
    name: str
    account_id: int = -1
    hidden_profile: bool = False
    stats: dict = None
    clan: str = None


class PotatoAlert:
    def __init__(self, ui):
        self.ui = ui
        self.config = Config()
        self.arena_info_file = os.path.join(self.config.replays_folder, 'tempArenaInfo.json')
        self.client_version = ''
        self.steam_version = False
        self.region = ''
        self.current_players = []
        self.last_started = 0.0
        self.api = ApiWrapper(self.config.api_key, self.config.region)

    async def run(self):
        while True:
            if not os.path.exists(self.arena_info_file):
                print('waiting for game start...')
            else:
                last_started = float(os.stat(os.path.join(self.config.replays_folder, 'tempArenaInfo.json')).st_mtime)
                if last_started > self.last_started:
                    self.last_started = last_started
                    print('new game started...')
                    self.read_arena_info()

                    for player in self.current_players:
                        try:
                            player.account_id = self.api.get_account_info(player.name)['data'][0]['account_id']
                            res = next(iter(self.api.get_player_stats(player.account_id)['data'].values()))

                            clan_id = next(iter(self.api.get_clan_info_player(player.account_id)['data'].values()))['clan_id']
                            if clan_id:
                                player.clan = self.api.get_clan_details(clan_id)['data'][str(clan_id)]['tag']

                            player.hidden_profile = res['hidden_profile']
                            if player.hidden_profile:
                                continue

                            player.stats = res['statistics']['pvp']
                        except InvalidApplicationIdError:
                            print('Invalid Application ID')
                            exit(1)
                    self.ui.fill_tables(self.current_players)
            await asyncio.sleep(10)

    def read_arena_info(self):
        arena_info = os.path.join(self.config.replays_folder, 'tempArenaInfo.json')
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
        self.endpoint = f'https://api.worldofwarships.{region}/wows/{{}}/{{}}/?'

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
        except ConnectionError:
            print('connection error')

    def get_account_info(self, name) -> dict:
        param = {
            'search': name,
            'type': 'exact',
            'fields': 'account_id'
        }
        return self.get_result('account', 'list', param)

    def statistics_of_players_ships(self, account_id: int, *,
                                    access_token: str = None,
                                    extra: str = None,
                                    fields: str = None,
                                    in_garage: bool = None,
                                    language: str = None,
                                    ship_id: int = 0) -> dict:

        if in_garage is not None:
            in_garage = '1' if in_garage else '0'
        param = {
            'account_id': account_id,
            'access_token': access_token,
            'extra': extra,
            'fields': fields,
            'in_garage': in_garage,
            'language': language,
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

    def get_clan_info_player(self, account_id: int) -> dict:
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


class Config(ConfigParser):
    def __init__(self):
        super().__init__()
        self.config_path = self.find_config()
        self.read_config()

    def __getattr__(self, item):
        try:
            return self['DEFAULT'][item]
        except KeyError as e:
            print(f'Keyerror: {e}')

    @staticmethod
    def find_config():
        if os.name == 'nt':
            return os.path.join(os.getenv('APPDATA'), 'PotatoAlert')
        if os.name == 'posix':
            return os.path.join(os.path.expanduser('~'), '.config/PotatoAlert')
        else:
            print('I have no idea which os you are on, please fix your shit')
            exit(1)

    def read_config(self):
        if not os.path.exists(self.config_path):
            os.makedirs(self.config_path)
        if not os.path.exists(os.path.join(self.config_path, 'config.ini')):
            self.create_default()
        with open(os.path.join(self.config_path, 'config.ini'), 'r') as configfile:
            self.read_file(configfile)

    def create_default(self):
        self['DEFAULT']['replays_folder'] = 'C:\\Program Files (x86)\\World_of_Warships_Eu\\replays'
        self['DEFAULT']['region'] = 'eu'
        self['DEFAULT']['api_key'] = '123'
        self.save()
        self.read_config()

    def save(self):
        with open(os.path.join(self.config_path, 'config.ini'), 'w') as configfile:
            self.write(configfile)


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


    # loop.run_until_complete(p.init_gui())
    # c = Config()
    # a = ApiWrapper(c.api_key, c.region)
    # print(a.statistics_of_players_ships(529548579))
    # print(a.get_player_stats(529548579))
    # print(a.get_account_id_by_name('nGu_RaZaq'))

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
from typing import List
from aiohttp import ClientSession
from aiohttp.client_exceptions import ClientResponseError, ClientError
from utils.config import Config
from dataclasses import dataclass
from gui import create_gui
from asyncqt import QEventLoop
from utils.api import ApiWrapper
from utils.api_errors import InvalidApplicationIdError
from utils.stat_colors import color_avg_dmg, color_battles, color_winrate
from version import __version__


@dataclass()
class Player:
    hidden_profile: bool
    team: int
    row: list
    colors: list
    class_ship: int  # for sorting we keep track of these too
    tier_ship: int
    nation_ship: int


class PotatoAlert:
    def __init__(self, ui):
        self.ui = ui
        self.config = Config()
        self.arena_info_file = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        self.client_version = ''
        self.steam_version = False
        self.region = ''
        self.last_started = 0.0
        self.api = ApiWrapper(self.config['DEFAULT']['api_key'], self.config['DEFAULT']['region'])
        self.invalid_api_key = asyncio.get_event_loop().run_until_complete(self.check_api_key())
        asyncio.get_event_loop().run_until_complete(self.check_version())

    async def check_version(self):
        try:
            url = 'https://raw.githubusercontent.com/razaqq/PotatoAlert/master/version.py'
            async with ClientSession() as s:
                async with s.get(url) as resp:
                    new_version = await resp.text()
                    new_version = new_version.split("'")[1]
            if __version__ != new_version:
                self.ui.notify_update(new_version)
        except ConnectionError:
            pass

    async def check_api_key(self):
        try:
            await self.api.get_account_info(0)
            return False
        except InvalidApplicationIdError:
            logging.error('The API key you provided is invalid, please go to the settings and check it!')
            return True

    async def reload_config(self):
        self.config = Config()
        self.arena_info_file = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        self.api = ApiWrapper(self.config['DEFAULT']['api_key'], self.config['DEFAULT']['region'])
        self.ui.config_reload_needed = False
        self.invalid_api_key = await self.check_api_key()

    async def run(self):
        if not self.invalid_api_key:
            logging.info('Waiting for match start...')
        while True:
            if self.ui.config_reload_needed:
                asyncio.get_event_loop().create_task(self.reload_config())
            if not self.invalid_api_key and os.path.exists(self.arena_info_file):
                last_started = float(os.stat(
                    os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')).st_mtime)
                if last_started != self.last_started:
                    logging.info('New game started, getting stats...')
                    await asyncio.sleep(0.05)
                    try:
                        players = await self.get_players(self.read_arena_info())
                        self.ui.fill_tables(players)
                        logging.info('Done.')
                        self.last_started = last_started
                    except InvalidApplicationIdError:
                        logging.error('The API Key you provided is invalid!')
                    except ConnectionError:
                        logging.error('Check your internet connection!')
                    except ClientResponseError as e:  # no idea what to do with these
                        logging.error(e)
                    except ClientError as e:
                        logging.error(e)
            await asyncio.sleep(5)

    async def get_players(self, data: List[dict]) -> List[Player]:
        players = []
        for p in data:
            player_name = p['name']
            ship_name = 'Error'
            battles, winrate, avg_dmg, winrate_ship, battles_ship = [0] * 5
            class_ship, nation_ship, tier_ship = [0] * 3

            account_search = await self.api.search_account(p['name'])
            account_id = account_search['data'][0]['account_id']

            team = p['relation']
            account_info = await self.api.get_account_info(account_id)
            account_info = account_info['data'][str(account_id)]
            hidden_profile = account_info['hidden_profile']

            ship_infos = await self.api.get_ship_infos(p['shipId'])
            ship = ship_infos['data'][str(p['shipId'])]

            clan = await self.api.get_player_clan(account_id)
            clan_data = clan['data'][str(account_id)]
            if clan_data and 'clan_id' in clan_data and clan_data['clan_id']:
                clan_id = clan_data['clan_id']
                clan_info = await self.api.get_clan_info(clan_id)
                clan_tag = clan_info['data'][str(clan_id)]['tag']
                player_name = f"[{clan_tag}] {p['name']}"

            # get general stats if profile is not private
            if not hidden_profile:
                stats = account_info['statistics']['pvp']
                if stats and 'battles' in stats:
                    battles = stats['battles']
                    if battles and 'wins' in stats and 'damage_dealt' in stats:  # at least one match
                        winrate = round(stats['wins'] / battles * 100, 1)
                        avg_dmg = int(round(stats['damage_dealt'] / battles, -2))

            if ship:
                ship_short_names = {
                    'Prinz Eitel Friedrich': 'P. E. Friedrich',
                    'Friedrich der Große': 'F. der Große',
                    'Admiral Graf Spee': 'A. Graf Spee',
                    'Oktyabrskaya Revolutsiya': 'Okt. Revolutsiya',
                    'HSF Admiral Graf Spee': 'HSF A. Graf Spee',
                    'Jurien de la Gravière': 'J. de la Gravière'
                }
                ship_name = ship_short_names.get(ship['name'], ship['name'])

                ship_sorting = {
                    'AirCarrier': 4,
                    'Battleship': 3,
                    'Cruiser': 2,
                    'Destroyer': 1
                }
                class_ship = ship_sorting[ship['type']]

                nation_sorting = {
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
                nation_ship = nation_sorting.get(ship['nation'], 0)

                tier_ship = ship['tier']

                if not hidden_profile:
                    ship_stats = await self.api.get_ship_stats(account_id, p['shipId'])
                    data = ship_stats['data'][str(account_id)]
                    if data:
                        ship_stats = data[0]['pvp']
                        if ship_stats and 'battles' in ship_stats:
                            battles_ship = ship_stats['battles']
                            if battles_ship and 'wins' in ship_stats:  # check that at least one match in ship
                                winrate_ship = round(ship_stats['wins'] / battles_ship * 100, 1)

            row = [player_name, ship_name]
            colors = [None, None]
            if not hidden_profile:
                row.extend([str(battles), str(winrate), str(avg_dmg)])
                colors.extend([color_battles(battles), color_winrate(winrate), color_avg_dmg(avg_dmg)])
                if ship_name != 'Error':
                    row.extend([str(battles_ship), str(winrate_ship)])
                    colors.extend([None, color_winrate(winrate_ship)])
            p = Player(hidden_profile, team, row, colors, class_ship, tier_ship, nation_ship)
            players.append(p)

        return sorted(players, key=lambda x: (x.class_ship, x.tier_ship, x.nation_ship), reverse=True)

    def read_arena_info(self) -> List[dict]:
        arena_info = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        if not os.path.exists(arena_info):
            return []
        with open(arena_info, 'r') as f:
            data = json.load(f)
            return [d for d in data['vehicles']]


if __name__ == '__main__':
    app, gui = create_gui()
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)
    gui.show()
    loop.run_until_complete(asyncio.sleep(1))

    pa = PotatoAlert(gui)
    loop.create_task(pa.run())
    with loop:
        sys.exit(loop.run_forever())

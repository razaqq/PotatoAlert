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
from typing import List, Tuple
from aiohttp import ClientSession, ClientTimeout
from aiohttp.client_exceptions import ClientResponseError, ClientError, ClientConnectionError, ServerTimeoutError
from utils.config import Config
from dataclasses import dataclass
import gui
from asyncqt import QEventLoop
from utils.api import ApiWrapper
from utils.api_errors import InvalidApplicationIdError
from utils.stat_colors import color_avg_dmg, color_battles, color_winrate, color_personal_rating
from utils.ship_utils import shorten_name, get_nation_sort, get_class_sort
from assets.colors import Grey
from utils import updater


@dataclass()
class Player:
    hidden_profile: bool
    team: int
    row: list
    colors: list
    class_ship: int  # for sorting we keep track of these too
    tier_ship: int
    nation_ship: int
    background: any = None


@dataclass()
class ArenaInfo:
    __slots__ = ['vehicles', 'mapId', 'mapName', 'matchGroup', 'ppt', 'scenario']
    vehicles: list
    mapId: int
    mapName: str
    matchGroup: str
    ppt: int
    scenario: str


class PotatoAlert:
    def __init__(self, ui):
        self.ui = ui
        self.config = Config()
        self.arena_info_file = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        self.client_version = ''
        self.steam_version = False
        self.region = ''
        self.last_started = 0.0
        self.setup_logger()
        self.api = ApiWrapper(self.config['DEFAULT']['api_key'], self.config['DEFAULT']['region'])
        self.invalid_api_key = asyncio.get_event_loop().run_until_complete(self.check_api_key())

    def setup_logger(self):
        logging.basicConfig(level=logging.ERROR,
                            format='%(asctime)s - %(levelname)-5s:  %(message)s',
                            datefmt='%H:%M:%S',
                            filename=os.path.join(self.config.config_path, 'Error.log'),
                            filemode='a')

    async def check_api_key(self):
        try:
            self.ui.update_status(1, 'Ready')
            await self.api.get_account_info(0)
            return False
        except InvalidApplicationIdError:
            self.ui.update_status(3, 'Invalid API')
            logging.error('The API key you provided is invalid, please go to the settings and check it!')
            return True
        except ClientConnectionError:
            logging.exception('Check your internet connection!')
            self.ui.update_status(3, 'Connection')

    async def reload_config(self):
        self.config = Config()
        self.arena_info_file = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        self.api = ApiWrapper(self.config['DEFAULT']['api_key'], self.config['DEFAULT']['region'])
        self.ui.config_reload_needed = False
        self.invalid_api_key = await self.check_api_key()

    async def run(self):
        while True:
            if self.ui.config_reload_needed:
                asyncio.get_event_loop().create_task(self.reload_config())
            if not self.invalid_api_key and os.path.exists(self.arena_info_file):
                last_started = float(os.stat(
                    os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')).st_mtime)
                if last_started != self.last_started:
                    await asyncio.sleep(0.05)
                    try:
                        arena_info = self.read_arena_info()
                        if self.config['DEFAULT'].getboolean('additional_info'):
                            self.ui.match_info.update_text(arena_info.mapName, arena_info.scenario, arena_info.ppt)
                        players = await self.get_players(arena_info.vehicles, arena_info.matchGroup)
                        self.ui.fill_tables(players)
                        self.ui.team_stats.update_text(*self.get_averages_and_colors(players))
                        self.ui.update_status(1, 'Ready')
                        self.last_started = last_started
                    except ClientConnectionError:
                        logging.exception('Check your internet connection!')
                        self.ui.update_status(3, 'Connection')
                    except (ClientError, ClientResponseError, Exception) as e:
                        logging.exception(e)
                        self.ui.update_status(3, 'Check Logs')
            await asyncio.sleep(5)

    async def get_players(self, player_data: List[dict], match_group: str) -> List[Player]:
        try:  # Get expected values for pr calculation from wows-numbers
            async with ClientSession(timeout=ClientTimeout(connect=10)) as s:
                async with s.get('https://api.wows-numbers.com/personal/rating/expected/json/') as resp:
                    expected = await resp.json()
        except (ClientConnectionError, ClientError, ClientResponseError, TimeoutError, ServerTimeoutError):
            expected = None

        players = []
        total = len(player_data)
        for p in player_data:
            self.ui.update_status(2, f'Getting stats ({round(len(players) / total * 100, 1)}%)')
            tasks = []
            player_name = p['name']
            team = p['relation']
            ship_name = 'Error'
            background = None
            battles, winrate, avg_dmg, winrate_ship, battles_ship = [0] * 5
            class_sort, nation_sort, tier_sort = [0] * 3

            if match_group == 'cooperative' and team == 2:  # COOP
                ship_infos = await self.api.get_ship_infos(p['shipId'])
                ship = ship_infos['data'][str(p['shipId'])]
                ship_name = shorten_name(ship['name'])
                class_sort = get_class_sort(ship['type'])  # these three are for sorting them in the board
                nation_sort = get_nation_sort(ship['nation'])
                tier_sort = ship['tier']
                p = Player(True, team, [player_name, ship_name], [None, None], class_sort, tier_sort, nation_sort, None)
                players.append(p)
                continue

            if (match_group == 'pve' or match_group == 'pve_premade') and team == 2:  # SCENARIO
                continue

            if match_group == 'clan':
                # TODO
                # Somehow get which server the clan is from and pull their stats from a different api endpoint
                pass

            try:  # try to get account id by searching by name, enter empty player if we get a KeyError
                account_search = await self.api.search_account(p['name'])
                account_id = account_search['data'][0]['account_id']
            except (KeyError, IndexError):
                p = Player(True, team, [player_name, ship_name], [None, None], class_sort, tier_sort, nation_sort, None)
                players.append(p)
                continue

            tasks.append(asyncio.ensure_future(self.api.get_account_info(account_id)))
            tasks.append(asyncio.ensure_future(self.api.get_player_clan(account_id)))
            tasks.append(asyncio.ensure_future(self.api.get_ship_infos(p['shipId'])))
            tasks.append(asyncio.ensure_future(self.api.get_ship_stats(account_id, p['shipId'])))
            tasks.append(asyncio.ensure_future(self.get_overall_personal_rating(account_id, expected)))
            responses = await asyncio.gather(*tasks)
            account_info = responses[0]
            clan = responses[1]
            ship_infos = responses[2]
            ship_stats = responses[3]
            pr = responses[4]

            try:  # get general account info and overall stats
                # account_info = await self.api.get_account_info(account_id)
                account_info = account_info['data'][str(account_id)]
                hidden_profile = account_info['hidden_profile']
                if not hidden_profile:
                    stats = account_info['statistics']['pvp']
                    if stats and 'battles' in stats:
                        battles = stats['battles']
                        if battles and stats['wins'] and stats['damage_dealt']:
                            # at least one match, otherwise we divide by 0
                            winrate = round(stats['wins'] / battles * 100, 1)
                            avg_dmg = int(round(stats['damage_dealt'] / battles, -2))
            except KeyError:
                hidden_profile = True

            try:  # get clan info and append clan tag to player name
                # clan = await self.api.get_player_clan(account_id)
                clan_data = clan['data'][str(account_id)]
                if clan_data and clan_data['clan']:
                    clan_tag = clan_data['clan']['tag']
                    player_name = f"[{clan_tag}]{p['name']}"
            except KeyError:
                pass

            try:
                # ship_infos = await self.api.get_ship_infos(p['shipId'])
                ship = ship_infos['data'][str(p['shipId'])]

                if ship:
                    ship_name = shorten_name(ship['name'])
                    class_sort = get_class_sort(ship['type'])  # these three are for sorting them in the board
                    nation_sort = get_nation_sort(ship['nation'])
                    tier_sort = ship['tier']

                    if not hidden_profile:
                        # ship_stats = await self.api.get_ship_stats(account_id, p['shipId'])
                        player_data = ship_stats['data'][str(account_id)]
                        if player_data:
                            ship_stats = player_data[0]['pvp']
                            if ship_stats and 'battles' in ship_stats:
                                battles_ship = ship_stats['battles']
                                if battles_ship and 'wins' in ship_stats:  # check that at least one match in ship
                                    winrate_ship = round(ship_stats['wins'] / battles_ship * 100, 1)
            except KeyError:
                pass

            # Get personal rating for background
            if not hidden_profile:
                # pr = await self.get_overall_personal_rating(account_id)
                if pr:
                    background = color_personal_rating(pr)

            # Put all stats in a dataclass
            row = [player_name, ship_name]
            colors = [None, None]
            if not hidden_profile:
                row.extend([str(battles), str(winrate), str(avg_dmg)])
                colors.extend([color_battles(battles), color_winrate(winrate), color_avg_dmg(avg_dmg)])
                if ship_name != 'Error':
                    row.extend([str(battles_ship), str(winrate_ship)])
                    colors.extend([None, color_winrate(winrate_ship)])
            p = Player(hidden_profile, team, row, colors, class_sort, tier_sort, nation_sort, background)
            players.append(p)

        return sorted(players, key=lambda x: (x.class_ship, x.tier_ship, x.nation_ship), reverse=True)

    async def get_overall_personal_rating(self, account_id: int = 0, expected=None):
        try:
            if not expected:
                return False
            total_pr = []
            stats = await self.api.get_ship_stats(account_id)
            total_battles = 0
            if stats:
                ship_stats = stats['data'][str(account_id)]
                if ship_stats:
                    for ship in ship_stats:
                        ship_id = str(ship['ship_id'])
                        battles = ship['pvp']['battles']
                        if ship_id in expected['data']:
                            s = expected['data'][ship_id]
                            if not s:
                                continue
                            expected_dmg = s['average_damage_dealt'] * battles
                            expected_wins = s['win_rate'] * battles
                            expected_frags = s['average_frags'] * battles
                            if not expected_frags or not expected_dmg or not expected_wins:
                                continue
                        else:
                            continue
                        r_dmg = ship['pvp']['damage_dealt'] / expected_dmg
                        r_wins = ship['pvp']['wins'] * 100 / expected_wins
                        r_frags = ship['pvp']['frags'] / expected_frags
                        n_dmg = max(0, (r_dmg - 0.4) / (1 - 0.4))
                        n_frags = max(0, (r_frags - 0.1) / (1 - 0.1))
                        n_wins = max(0, (r_wins - 0.7) / (1 - 0.7))
                        pr = 700 * n_dmg + 300 * n_frags + 150 * n_wins
                        total_pr.append(pr * ship['pvp']['battles'])
                        total_battles += ship['pvp']['battles']
            return sum(total_pr) / total_battles
        except (KeyError, IndexError, ZeroDivisionError):
            return False

    @staticmethod
    def get_averages_and_colors(players: List[Player]):
        t1_wr = [float(p.row[3]) for p in players if p.team == 1 or p.team == 0 and not p.hidden_profile]
        t1_dmg = [int(p.row[4]) for p in players if p.team == 1 or p.team == 0 and not p.hidden_profile]
        t2_wr = [float(p.row[3]) for p in players if p.team == 2 and not p.hidden_profile]
        t2_dmg = [int(p.row[4]) for p in players if p.team == 2 and not p.hidden_profile]

        t1_wr_c = color_winrate(sum(t1_wr) / len(t1_wr)) if t1_wr else Grey()
        t1_dmg_c = color_avg_dmg(sum(t1_dmg) / len(t1_dmg)) if t1_dmg else Grey()
        t2_wr_c = color_winrate(sum(t2_wr) / len(t2_wr)) if t2_wr else Grey()
        t2_dmg_c = color_avg_dmg(sum(t2_dmg) / len(t2_dmg)) if t2_dmg else Grey()

        t1_wr = round(sum(t1_wr) / len(t1_wr), 1) if t1_wr else 0.0
        t1_dmg = int(round(sum(t1_dmg) / len(t1_dmg), -2)) if t1_dmg else 0
        t2_wr = round(sum(t2_wr) / len(t2_wr), 1) if t2_wr else 0.0
        t2_dmg = int(round(sum(t2_dmg) / len(t2_dmg), -2)) if t2_dmg else 0

        return t1_wr, t1_wr_c, t1_dmg, t1_dmg_c, t2_wr, t2_wr_c, t2_dmg, t2_dmg_c

    def read_arena_info(self) -> ArenaInfo:
        arena_info = os.path.join(self.config['DEFAULT']['replays_folder'], 'tempArenaInfo.json')
        if not os.path.exists(arena_info):
            return None
        with open(arena_info, 'r') as f:
            data = json.load(f)
            a = ArenaInfo([d for d in data['vehicles']], data['mapId'], data['mapDisplayName'], data['matchGroup'],
                          data['playersPerTeam'], data['scenario'])
            return a
            # return [d for d in data['vehicles']], data['matchGroup']


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    update = loop.run_until_complete(updater.check_update())
    if update:
        app, gui = updater.create_gui()
    else:
        app, gui = gui.create_gui()
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)
    loop.run_until_complete(asyncio.sleep(0.1))
    if update:
        loop.run_until_complete(gui.update_progress(updater.update))
        sys.exit(0)

    pa = PotatoAlert(gui)
    loop.create_task(pa.run())
    with loop:
        sys.exit(loop.run_forever())

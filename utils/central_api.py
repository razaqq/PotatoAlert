from typing import Tuple, List
import aiohttp
import json
from utils.dcs import Player, Team


class CentralApi:
    def __init__(self, config, pa):
        self.config = config
        self.pa = pa

    async def get_players(self, arena_info) -> Tuple[List[Player], List[Player], Tuple[Team, Team]]:
        session = aiohttp.ClientSession(conn_timeout=10)
        try:
            async with session.ws_connect('ws://www.perry-swift.de:33333') as ws:
                self.pa.signals.status.emit(2, f'Loading')

                await ws.send_str(json.dumps(arena_info))
                match = await ws.receive_json()

                team1, team2 = [], []
                for t_id, team in enumerate((match['Team1'], match['Team2'])):
                    if match['MatchGroup'] in ['pve', 'pve_premade'] and t_id == 1:
                        continue
                    for p in team['Players']:
                        values, colors = [], []
                        if p['Clan'] and match['MatchGroup'] != 'clan':
                            clan_tag = p['Clan']['Tag']
                            clan_color = p['Clan']['Color']
                        else:
                            clan_tag = ''
                            clan_color = ''
                        values += [p['Name']]
                        colors += [p['NameColor']]
                        if p['Ship']:
                            values.append(p['Ship']['Name'])
                            colors.append(p['Ship']['Color'])
                        if not p['HiddenPro'] and not (match['MatchGroup'] == 'cooperative' and t_id == 1):
                            values += [p['Battles'], self.to_fixed_str(p['WinRate']), p['AvgDmg'], p['BattlesShip'],
                                       self.to_fixed_str(p['WRShip']), p['AvgDmgShip']]
                            colors += [p['BattlesC'], p['WinRateC'], p['AvgDmgC'], p['BattlesShipC'], p['WRShipC'],
                                       p['AvgDmgShipC']]
                        player = Player(p['AccountID'], p['HiddenPro'], -1, values, colors,
                                        -1, -1, -1, clan_tag, p['PrC'], clan_color, match['Region'])
                        t = team1 if t_id == 0 else team2
                        t.append(player)

                t1_avg, t2_avg = Team(), Team()
                t1_avg.winrate = self.to_fixed_str(match['Team1']['AvgWR'])
                t1_avg.avg_dmg = match['Team1']['AvgDmg']
                t1_avg.winrate_c = match['Team1']['AvgWRC']
                t1_avg.avg_dmg_c = match['Team1']['AvgDmgC']
                if match['MatchGroup'] not in ['cooperative', 'pve', 'pve_premade']:
                    t2_avg.winrate = self.to_fixed_str(match['Team2']['AvgWR'])
                    t2_avg.avg_dmg = match['Team2']['AvgDmg']
                    t2_avg.winrate_c = match['Team2']['AvgWRC']
                    t2_avg.avg_dmg_c = match['Team2']['AvgDmgC']

                clans = False
                if match['MatchGroup'] == 'clan' and match['Team1']['Players'] and match['Team2']['Players']:
                    t1_clan = match['Team1']['Players'][0]['Clan']
                    t2_clan = match['Team2']['Players'][0]['Clan']
                    if t1_clan and t2_clan:
                        c1 = (t1_clan['Name'], t1_clan['Tag'], t1_clan['Color'])
                        c2 = (t2_clan['Name'], t2_clan['Tag'], t2_clan['Color'])
                        self.pa.servers = (match['Region'], t2_clan['Region'])
                        self.pa.signals.servers.emit()
                        self.pa.clans = (c1, c2)
                        self.pa.signals.clans.emit()
                        clans = True
                if not clans:
                    self.pa.servers = (None, None)
                    self.pa.signals.servers.emit()
                    self.pa.clans = (None, None)
                    self.pa.signals.clans.emit()

                return team1, team2, (t1_avg, t2_avg)
        finally:
            await session.close()

    @staticmethod
    def to_fixed_str(value: float, digits: int = 1) -> str:
        if type(value) == float or type(value) == int:
            value = f'{value:.{digits}f}'
        return value

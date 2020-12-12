from typing import Tuple, List
import aiohttp
import json
from utils.dcs import Player, Team


class CentralApi:
    def __init__(self, config, pa):
        self.config = config
        self.pa = pa

    async def get_players(self, arena_info) -> Tuple[List[Player], List[Player], Tuple[Team, Team]]:
        timeout = aiohttp.ClientTimeout(total=60, sock_read=20, sock_connect=20, connect=20)
        session = aiohttp.ClientSession(timeout=timeout)
        try:
            async with session.ws_connect('ws://www.perry-swift.de:33333') as ws:
                self.pa.signals.status.emit(2, f'Loading')

                await ws.send_str(json.dumps(arena_info))
                match = await ws.receive_json()

                team1, team2 = [], []
                for t_id, team in enumerate((match['team1'], match['team2'])):
                    if match['matchGroup'] in ['pve', 'pve_premade'] and t_id == 1:
                        continue
                    for p in team['players']:
                        values, colors = [], []
                        if p['clan'] and match['matchGroup'] != 'clan':
                            clan_tag = p['clan']['tag']
                            clan_color = p['clan']['color']
                        else:
                            clan_tag = ''
                            clan_color = ''
                        values += [p['name']]
                        colors += [p['nameColor']]
                        if p['ship']:
                            values.append(p['ship']['name'])
                            colors.append(p['ship']['color'])
                        if not p['hiddenPro'] and not (match['matchGroup'] == 'cooperative' and t_id == 1):
                            values += [p['battles']['string'], p['winrate']['string'], p['avgDmg']['string'],
                                       p['battlesShip']['string'], p['winrateShip']['string'],
                                       p['avgDmgShip']['string']]
                            colors += [p['battles']['color'], p['winrate']['color'], p['avgDmg']['color'],
                                       p['battlesShip']['color'], p['winrateShip']['color'], p['avgDmgShip']['color']]
                        player = Player(0, p['hiddenPro'], -1, values, colors,
                                        -1, -1, -1, clan_tag, p['prColor'], clan_color, match['region'])
                        t = team1 if t_id == 0 else team2
                        t.append(player)

                t1_avg, t2_avg = Team(), Team()
                t1_avg.winrate = match['team1']['avgWr']['string']
                t1_avg.avg_dmg = match['team1']['avgDmg']['string']
                t1_avg.winrate_c = match['team1']['avgWr']['color']
                t1_avg.avg_dmg_c = match['team1']['avgDmg']['color']
                if match['matchGroup'] not in ['cooperative', 'pve', 'pve_premade']:
                    t2_avg.winrate = match['team2']['avgWr']['string']
                    t2_avg.avg_dmg = match['team2']['avgDmg']['string']
                    t2_avg.winrate_c = match['team2']['avgWr']['color']
                    t2_avg.avg_dmg_c = match['team2']['avgDmg']['color']

                clans = False
                if match['matchGroup'] == 'clan' and match['team1']['players'] and match['team2']['players']:
                    t1_clan = match['team1']['players'][0]['clan']
                    t2_clan = match['team2']['players'][0]['clan']
                    if t1_clan and t2_clan:
                        c1 = (t1_clan['name'], t1_clan['tag'], t1_clan['color'])
                        c2 = (t2_clan['name'], t2_clan['tag'], t2_clan['color'])
                        self.pa.servers = (match['region'], t2_clan['region'])
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

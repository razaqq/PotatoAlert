import logging
from typing import Tuple, List
import websockets
import json
from utils.dcs import Player, Team


class CentralApi:
    def __init__(self, config, pa):
        self.config = config
        self.pa = pa

    async def get_players(self, arena_info) -> Tuple[List[Player], List[Player], Tuple[Team, Team]]:
        async with websockets.connect('ws://127.0.0.1:10000') as websocket:
            try:
                self.pa.signals.status.emit(2, f'Loading')
                await websocket.send(json.dumps(arena_info))
                res = await websocket.recv()
                match = json.loads(res)

                team1, team2 = [], []
                for t_id, team in enumerate((match['Team1'], match['Team2'])):
                    for p in team['Players']:
                        values, colors = [], []
                        clan_tag = p['Clan']['Tag'] if p['Clan'] else ''
                        clan_color = p['Clan']['Color'] if p['Clan'] else ''
                        values += [p['Name']]
                        colors += [p['NameColor']]
                        if p['Ship']:
                            values.append(p['Ship']['Name'])
                            colors.append(p['Ship']['Color'])
                        if not p['HiddenPro']:
                            values += [p['Battles'], p['WinRate'], p['AvgDmg'], p['BattlesShip'], p['WRShip'], p['AvgDmgShip']]
                            colors += [p['BattlesC'], p['WinRateC'], p['AvgDmgC'], p['BattlesShipC'], p['WRShipC'], p['AvgDmgShipC']]
                        player = Player(p['AccountID'], p['HiddenPro'], -1, values, colors,
                                        -1, -1, -1, clan_tag, p['PrC'], clan_color, match['Region'])
                        t = team1 if t_id == 0 else team2
                        t.append(player)

                t1_avg, t2_avg = Team(), Team()
                t1_avg.winrate = match['Team1']['AvgWR']
                t1_avg.avg_dmg = match['Team1']['AvgDmg']
                t1_avg.winrate_c = match['Team1']['AvgWRC']
                t1_avg.avg_dmg_c = match['Team1']['AvgDmgC']

                t2_avg.winrate = match['Team2']['AvgWR']
                t2_avg.avg_dmg = match['Team2']['AvgDmg']
                t2_avg.winrate_c = match['Team2']['AvgWRC']
                t2_avg.avg_dmg_c = match['Team2']['AvgDmgC']

                return team1, team2, (t1_avg, t2_avg)
            except ConnectionRefusedError:
                logging.exception('Connection refused by remote host!')
                self.pa.signals.status.emit(3, 'Connection')
            except websockets.exceptions.InvalidMessage:
                logging.exception('Did not receive valid response!')
                self.pa.signals.status.emit(3, 'Response')
            except websockets.exceptions.ConnectionClosedError:
                logging.exception('Connection was closed by remote host!')
                self.pa.signals.status.emit(3, 'Connection')
            except EOFError:
                logging.exception('Connection was closed by remote host!')
                self.pa.signals.status.emit(3, 'EOFError')
            finally:
                await websocket.close()

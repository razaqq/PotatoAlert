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

from dataclasses import dataclass
from assets.colors import QColor, Grey


@dataclass()
class Player:
    hidden_profile: bool
    team: int
    row: list
    colors: list
    class_ship: int  # for sorting we keep track of these too
    tier_ship: int
    nation_ship: int
    clan_tag: str
    background: any = None
    clan_color: any = None


@dataclass()
class ArenaInfo:
    __slots__ = ['vehicles', 'mapId', 'mapName', 'matchGroup', 'ppt', 'scenario']
    vehicles: list
    mapId: int
    mapName: str
    matchGroup: str
    ppt: int
    scenario: str


@dataclass()
class Team:
    winrate: float = 0.0
    avg_dmg: int = 0
    matches: int = 0
    winrate_c: QColor = Grey()
    avg_dmg_c: QColor = Grey()

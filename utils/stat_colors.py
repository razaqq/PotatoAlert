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

from assets.colors import Purple, Pink, Cyan, DGreen, LGreen, Yellow, Orange, Red, DGrey


def color_winrate(wr: float, battles_ship: int = 1) -> list:
    if not battles_ship:
        return DGrey
    c = Purple if wr >= 65 else Pink if wr >= 60 else Cyan if wr >= 56 else DGreen if wr >= 54 else \
        LGreen if wr >= 52 else Yellow if wr >= 49 else Orange if wr >= 47 else Red
    return c


def color_battles(battles: int) -> list:
    c = Purple if battles >= 20000 else Cyan if battles >= 14000 else LGreen if battles >= 9000 else \
        Yellow if battles >= 5000 else Orange if battles >= 2000 else Red
    return c


def color_avg_dmg(avg_dmg: float) -> list:
    c = Pink if avg_dmg >= 48500 else Cyan if avg_dmg >= 38000 else LGreen if avg_dmg >= 33000 else \
        Yellow if avg_dmg >= 22000 else Orange if avg_dmg >= 16000 else Red
    return c


def color_personal_rating(pr: float) -> list:
    c = Purple + [50] if pr > 2450 else Pink + [50] if pr > 2100 else Cyan + [75] if pr > 1750 else \
        DGreen + [50] if pr > 1550 else LGreen + [50] if pr > 1350 else Yellow + [50] if pr > 1100 else \
        Orange + [50] if pr > 750 else Red + [50]
    return c


def color_avg_dmg_ship(avg_dmg: float, ship_id: str, color_limits: dict, battles_ship: int = 1) -> list:
    if not battles_ship:
        return DGrey
    if ship_id in color_limits:
        limits = [x['value'] for x in color_limits[ship_id]]
        c = Red if avg_dmg < limits[0] else Orange if avg_dmg < limits[1] else \
            Yellow if avg_dmg < limits[2] else LGreen if avg_dmg < limits[3] else \
            Cyan if avg_dmg < limits[4] else Pink
        return c
    return DGrey

from assets.colors import Purple, Pink, Cyan, DGreen, LGreen, Yellow, Orange, Red


def color_winrate(wr):
    c = Purple() if wr >= 65 else Pink() if wr >= 60 else Cyan() if wr >= 56 else DGreen() if wr >= 54 else \
        LGreen() if wr >= 52 else Yellow() if wr >= 49 else Orange() if wr >= 47 else Red()
    return c


def color_battles(battles):
    c = Purple() if battles >= 20000 else Cyan() if battles >= 14000 else LGreen() if battles >= 9000 else \
        Yellow() if battles >= 5000 else Orange() if battles >= 2000 else Red()
    return c


def color_avg_dmg(avg_dmg):
    c = Pink() if avg_dmg >= 48500 else Cyan() if avg_dmg >= 38000 else LGreen() if avg_dmg >= 33000 else \
        Yellow() if avg_dmg >= 22000 else Orange() if avg_dmg >= 16000 else Red()
    return c


def color_personal_rating(pr):
    c = Purple(100) if pr > 2450 else Pink(100) if pr > 2100 else Cyan(100) if pr > 1750 else \
        DGreen(100) if pr > 1550 else LGreen(100) if pr > 1350 else Yellow(100) if pr > 1100 else \
        Orange(100) if pr > 750 else Red(100)
    return c

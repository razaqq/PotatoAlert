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

from PyQt5.QtGui import QColor


def hex_to_rgb(hex_code):
    hex_code = hex_code.lstrip('#')
    hlen = len(hex_code)
    return tuple(int(hex_code[i:i + hlen // 3], 16) for i in range(0, hlen, hlen // 3))


# Ship specific stats could be pulled from https://api.wows-numbers.com/colors/json/
red = hex_to_rgb('#FE0E00')
orange = hex_to_rgb('#FE7903')
yellow = hex_to_rgb('#FFC71F')
lgreen = hex_to_rgb('#17D133')  # default #44B300
dgreen = hex_to_rgb('#017E14')  # default #318000
cyan = hex_to_rgb('#02C9B3')
pink = hex_to_rgb('#D042F3')
purple = hex_to_rgb('#A00DC5')


class Orange(QColor):
    def __init__(self, alpha=255):
        super().__init__(*orange, alpha)


class Purple(QColor):
    def __init__(self, alpha=255):
        super().__init__(*purple, alpha)


class Pink(QColor):
    def __init__(self, alpha=255):
        super().__init__(*pink, alpha)


class Cyan(QColor):
    def __init__(self, alpha=255):
        super().__init__(*cyan, alpha)


class Red(QColor):
    def __init__(self, alpha=255):
        super().__init__(*red, alpha)


class LGreen(QColor):
    def __init__(self, alpha=255):
        super().__init__(*lgreen, alpha)


class DGreen(QColor):
    def __init__(self, alpha=255):
        super().__init__(*dgreen, alpha)


class Grey(QColor):
    def __init__(self, alpha=255):
        super().__init__(224, 222, 218, alpha)


class Yellow(QColor):
    def __init__(self, alpha=255):
        super().__init__(*yellow, alpha)


class White(QColor):
    def __init__(self, alpha=255):
        super().__init__(255, 255, 255, alpha)

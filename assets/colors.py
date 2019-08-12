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


class Orange(QColor):
    def __init__(self, alpha=255):
        super().__init__(255, 149, 0, alpha)


class Purple(QColor):
    def __init__(self, alpha=255):
        super().__init__(124, 8, 130, alpha)


class Pink(QColor):
    def __init__(self, alpha=255):
        super().__init__(222, 37, 232, alpha)


class Cyan(QColor):
    def __init__(self, alpha=255):
        super().__init__(63, 224, 214, alpha)


class Red(QColor):
    def __init__(self, alpha=255):
        super().__init__(199, 10, 10, alpha)


class LGreen(QColor):
    def __init__(self, alpha=255):
        super().__init__(23, 209, 51, alpha)


class DGreen(QColor):
    def __init__(self, alpha=255):
        super().__init__(1, 126, 20, alpha)


class Grey(QColor):
    def __init__(self, alpha=255):
        super().__init__(224, 222, 218, alpha)


class Yellow(QColor):
    def __init__(self, alpha=255):
        super().__init__(255, 208, 18, alpha)


class White(QColor):
    def __init__(self, alpha=255):
        super().__init__(255, 255, 255, alpha)

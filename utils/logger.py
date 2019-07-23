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

from logging import Handler, Formatter, DEBUG
from PyQt5.QtWidgets import QTextEdit
from PyQt5.QtCore import Qt


class Logger(Handler):
    def __init__(self, parent):
        super().__init__(level=DEBUG)
        self.widget = QTextEdit(parent)
        self.widget.setReadOnly(True)
        self.widget.resize(parent.size())
        self.widget.setFocusPolicy(Qt.NoFocus)
        self.widget.setMouseTracking(False)
        self.setFormatter(Formatter('%(asctime)s - %(levelname)-5s:  %(message)s', datefmt='%H:%M:%S'))
        self.colors = {
            'DEBUG': '#00ffec',
            'INFO': '#ffffff',
            'WARNING': '#ffe700',
            'ERROR': '#ff0000'
        }

    def emit(self, record):
        msg = self.format(record)
        html = f"<span style=\" font-size:8pt; font-weight:400; color:{self.colors[record.levelname]};\" >{msg}</span>"
        self.widget.append(html)

import os
from configparser import ConfigParser

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


class Config(ConfigParser):
    def __init__(self):
        super().__init__()
        self.config_path = self.find_config()
        self.read_config()

    @staticmethod
    def find_config():
        if os.name == 'nt':
            return os.path.join(os.getenv('APPDATA'), 'PotatoAlert')
        if os.name == 'posix':
            return os.path.join(os.path.expanduser('~'), '.config/PotatoAlert')
        else:
            print('I have no idea which os you are on, please fix your shit')
            exit(1)

    def read_config(self):
        if not os.path.exists(self.config_path):
            os.makedirs(self.config_path)
        if not os.path.exists(os.path.join(self.config_path, 'config.ini')):
            self.create_default()
        with open(os.path.join(self.config_path, 'config.ini'), 'r') as configfile:
            self.read_file(configfile)

    def create_default(self):
        self['DEFAULT']['replays_folder'] = ''
        self['DEFAULT']['region'] = 'eu'
        self['DEFAULT']['api_key'] = '123'
        self['DEFAULT']['theme'] = '0'
        self.save()
        self.read_config()

    def save(self):
        with open(os.path.join(self.config_path, 'config.ini'), 'w') as configfile:
            self.write(configfile)

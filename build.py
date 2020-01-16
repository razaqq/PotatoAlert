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

import os
import sys
import subprocess


if __name__ == '__main__':
    root = os.path.abspath(os.path.dirname(__file__))
    main = os.path.join(root, 'potatoalert.py')
    assets = os.path.join(root, 'assets')
    icon = os.path.join(assets, 'potato.ico')
    assets_sep = ':' if os.name == 'posix' else ';'

    debug_flags = '-F -y -d imports -d bootloader'
    build_flags = '-F -y -w'

    build = f'{sys.executable} -m PyInstaller {debug_flags} -i "{icon}" --add-data "{assets}"{assets_sep}"assets/" ' \
            f'"{main}"'

    subprocess.call(
        build
    )

    if os.path.exists(os.path.join(root, 'dist', 'potatoalert_x64.exe')):
        os.remove(os.path.join(root, 'dist', 'potatoalert_x64.exe'))
    os.rename(os.path.join(root, 'dist', 'potatoalert.exe'), os.path.join(root, 'dist', 'potatoalert_x64.exe'))
    os.remove(os.path.join(root, 'potatoalert.spec'))

import os
import sys
import subprocess

ADDITIONAL_LIB_PATHS = 'C:\\Program Files (x86)\\Windows Kits\\10\\Redist\\ucrt\\DLLs\\x64'
QT5_LIBS_PATH = 'C:\\Users\\User\\AppData\\Local\\Programs\\Python\\Python37\\Lib\\site - packages\\PyQt5\\Qt\\bin'


if __name__ == '__main__':
    root = os.path.abspath(os.path.dirname(__file__))
    main = os.path.join(root, 'potatoalert.py')
    assets = os.path.join(root, 'assets')
    icon = os.path.join(assets, 'potato.ico')
    assets_sep = ':' if os.name == 'posix' else ';'

    debug_flags = '-F -y'
    build_flags = '-F -y -w'

    build = f'{sys.executable} -m PyInstaller {build_flags} -i "{icon}" --add-data "{assets}"{assets_sep}"assets/" ' \
            f'--paths "{ADDITIONAL_LIB_PATHS}" --paths "{QT5_LIBS_PATH}" "{main}"'

    subprocess.call(
        build
    )

    os.remove(os.path.join(root, 'potatoalert.spec'))

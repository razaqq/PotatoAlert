import os
import subprocess
from shutil import rmtree


PYTHONPATH = 'python'
ADDITIONAL_LIB_PATHS = 'C:\\Program Files (x86)\\Windows Kits\\10\\Redist\\ucrt\\DLLs\\x64'
QT5_LIBS_PATH = 'C:\\Users\\User\\AppData\\Local\\Programs\\Python\\Python37\\Lib\\site - packages\\PyQt5\\Qt\\bin'


if __name__ == '__main__':
    root = os.path.abspath(os.path.dirname(__file__))
    main = os.path.join(root, 'potatoalert.py')
    assets = os.path.join(root, 'assets')
    icon = os.path.join(assets, 'potato.ico')

    build = f'pyinstaller -y -w -i "{icon}" --add-data "{assets}";"assets/" --paths "{ADDITIONAL_LIB_PATHS}" ' \
            f'--paths "{QT5_LIBS_PATH}" "{main}"'

    subprocess.call(
        build
    )

    rmtree(os.path.join(root, 'dist'))
    os.remove(os.path.join(root, 'potatoalert.spec'))

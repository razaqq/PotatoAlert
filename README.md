# PotatoAlert

## Use pre-compiled build
You can find pre-compiled builds for Windows and Linux [here](https://github.com/razaqq/PotatoAlert/releases).


## Run with python without compiling
Otherwise you can run it directly with python 3.7.
For that you will need a few modules:

Dependencies:
- PyQt5
- asyncqt
- pyqtconfig (https://github.com/mjirik/pyqtconfig)
- PyQtWebEngine
- requests

## Compile yourseelf
1. Get all the dependencies listed above except PyQt5
2. Get PyQt5 version 5.12.1, I was getting lib issues with any newer version.
3. Get pywin32 and pypiwin32
4. Get PyInstaller
5. Open build.py and edit the lines at the top accordingly. Then run it.
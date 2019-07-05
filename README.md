# PotatoAlert

## Screenshots

![default](screens/default.png?raw=true)
![default](screens/dark.png?raw=true)

## Use pre-compiled build
You can find pre-compiled builds for Windows and Linux [here](https://github.com/razaqq/PotatoAlert/releases).

1. Just start the binary, there is no install.
2. Go [here](https://developers.wargaming.net/applications/) and create an application for WoWs. Enter the API key in the settings.
3. Set your replays folder in the settings.


## Run with python without compiling
Otherwise you can run it directly with python 3.7.
For that you will need a few modules:

Dependencies:
- PyQt5
- asyncqt
- pyqtconfig (https://github.com/mjirik/pyqtconfig)
- PyQtWebEngine
- requests

## Compile yourself
1. Get all the dependencies listed above except PyQt5
2. Get PyQt5 version 5.12.1, I was getting lib issues with any newer version.
3. Get pywin32 and pypiwin32
4. Get https://pypi.org/project/auto-py-to-exe/
5. add --paths "PYTHON/Lib/site - packages/PyQt5/Qt/bin" manual entry, replace PYTHON with the path to your python install
6. add /assets as additional files
7. compile
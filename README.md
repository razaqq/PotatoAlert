# PotatoAlert


[![version](https://img.shields.io/github/v/release/razaqq/PotatoAlert.svg?style=flat-square)](https://github.com/razaqq/PotatoAlert/releases) 
[![Github all releases](https://img.shields.io/github/downloads/razaqq/PotatoAlert/total.svg?style=flat-square)](https://github.com/razaqq/PotatoAlert/releases)
[![appveyor build status](https://img.shields.io/appveyor/build/razaqq/PotatoAlert?style=flat-square&logo=appveyor)](https://ci.appveyor.com/project/razaqq/PotatoAlert)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT)

## Screenshots

![default](https://i.imgur.com/ra4l1Kd.png)

## Features
- Compact layout, so should easily fit on your second monitor
- Stats are colored based on wows-numbers.com, they are always taken from game mode "randoms"
- Backgrounds are colored by player's overall personal rating
- You can choose between using a central API server, or providing your own API key
- Double clicking a player will bring up his wows-numbers.com profile
- Support for all game modes, even clan wars with teams from different servers

## Warning
This tool is by no means meant for stat shaming or being toxic towards other players in general.
Please behave yourself in chat.
If you think you cannot follow this simple rule, then you hereby don't have my permission to use this tool.

## Installation
You have 3 options to install/use PotatoAlert.

#### Use pre-compiled build (recommended)
You can find pre-compiled builds for Windows [here](https://github.com/razaqq/PotatoAlert/releases).

1. Just start the binary, there is no install.
2. Set your replays folder in the settings.
3. (Optional) If you want to use your own API key for loading stats, go [here](https://developers.wargaming.net/applications/) and create an application for WoWs. Enter the API key in the settings. This will result in longer loading times.



#### Run with python without compiling
If you dont want to use a compiled binary, it is possible to run it directly with python>=3.7.
For that you will need to install a few dependencies:
```console
python -m pip install -r requirements.txt
```
I would recommend doing this in a virtualenv.

#### Compile yourself
1. Get dependencies like [here](#Run-with-python-without-compiling).
2. Get pywin32-ctypes by running `python -m pip install pywin32-ctypes`
3. Get PyInstaller by running `python -m pip install PyInstaller`
4. Run build.py to compile `python build.py`

You will find your binary in `dist/`

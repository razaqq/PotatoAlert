# PotatoAlert


[![version](https://img.shields.io/github/v/release/razaqq/PotatoAlert.svg?style=flat-square)](https://github.com/razaqq/PotatoAlert/releases) 
[![Github all releases](https://img.shields.io/github/downloads/razaqq/PotatoAlert/total.svg?style=flat-square)](https://github.com/razaqq/PotatoAlert/releases)
[![appveyor build status](https://img.shields.io/github/workflow/status/razaqq/potatoalert/Build?label=actions&logo=github&style=flat-square)](https://github.com/razaqq/PotatoAlert/actions/workflows/build.yaml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Discord Chat](https://img.shields.io/discord/711953820745203815.svg?&logo=discord&logoColor=ffffff&style=flat-square)](https://discord.gg/Ut8t8PA)
[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg?&logo=paypal&style=flat-square)](https://paypal.me/potatoalert)

## Screenshots

![default](.github/potatoalert.png)

## Features
- Compact layout, so should easily fit on your second monitor
- Stats are colored based on wows-numbers.com, they are always taken from game mode "randoms"
- Backgrounds are colored by player's overall personal rating
- Double clicking a player will bring up his wows-numbers.com profile
- Support for all game modes, even clan wars with teams from different servers

## Warning
This tool is by no means meant for stat shaming or being toxic towards other players in general.
Please behave yourself in chat.
If you think you cannot follow this simple rule, then you hereby don't have my permission to use this tool.

## Compiling
#### Requirements
- Qt >= 5.15.0 (Qt6 currently not supported)
- clang >= 11.0.0
- ninja >= 1.10.2
- cmake >= 3.17
- Visual Studio 2019
- Windows SDK >= 10
- python >= 3.7

#### Steps
- Get Paths
    - Qt5 `-DCMAKE_PREFIX_PATH=C:\Qt\5.15.0\msvc2019_64`
    - add clang to PATH 
      - `set CC=clang`
      - `set CXX=clang++`
    - Qt IFW `-DCPACK_IFW_ROOT=C:/Qt/Tools/QtInstallerFramework` (only for building the installer)
- Call cmake
```console
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\5.15.0\msvc2019_64 -DCMAKE_RC_COMPILER=RC
cmake --build build --config Release --target PotatoAlert PotatoUpdater
```
- You find the build output in `.\build\`
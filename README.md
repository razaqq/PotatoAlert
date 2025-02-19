# PotatoAlert

[![version](https://img.shields.io/github/v/release/razaqq/PotatoAlert.svg?style=flat-square)](https://github.com/razaqq/PotatoAlert/releases)
[![Github all releases](https://img.shields.io/github/downloads/razaqq/PotatoAlert/total.svg?style=flat-square)](https://github.com/razaqq/PotatoAlert/releases)
[![actions build status](https://img.shields.io/github/actions/workflow/status/razaqq/potatoalert/build.yaml?label=actions&logo=github&style=flat-square)](https://github.com/razaqq/PotatoAlert/actions/workflows/build.yaml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Discord Chat](https://img.shields.io/discord/711953820745203815.svg?&logo=discord&logoColor=ffffff&style=flat-square)](https://discord.gg/Ut8t8PA)
[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg?&logo=paypal&style=flat-square)](https://paypal.me/potatoalert)

## Screenshots

![default](.github/screen1.png)
![default](.github/screen2.png)
![default](.github/screen3.png)

## Features

- Fetch stats of current match automatically when your game starts
- Match history to record all played matches
- Replay parser to extract information from replays automatically (into match history)
- Compact layout (either vertical or horizontal), so should easily fit on your second monitor
- Stats and backgrounds (overall personal rating) are colored based on wows-numbers.com
- Double clicking a player will bring open the wows-numbers.com profile
- Support for all game modes, even clan wars with teams from different servers

## Warning

This tool is by no means meant for stat shaming or being toxic towards other players in general.
Please behave yourself in chat.
If you think you cannot follow this simple rule, then you hereby don't have my permission to use this tool.

## Installing

### Windows

Download the [latest Release](https://github.com/razaqq/PotatoAlert/releases/latest), unpack it, start it.

### Linux

If you are on Arch Linux, there exists an [AUR package](https://aur.archlinux.org/packages/potatoalert). If you are on another distro, you have to compile from source.

## Compiling

### Windows

#### Requirements

- [conan](https://conan.io/) >= 2.0
- [Qt](https://www.qt.io/) >= 6.6.0
- [clang](https://clang.llvm.org/) >= 11.0.0 or MSVC
- [ninja](https://ninja-build.org/) >= 1.10.2
- [cmake](https://cmake.org/) >= 3.17
- [Visual Studio 2022](https://visualstudio.microsoft.com)
- Windows SDK >= 10
- [python](https://www.python.org/) >= 3.7
- [rust](https://www.rust-lang.org/) >= 1.60

#### Steps

- [Setup a conan profile for clang-cl](https://docs.conan.io/2/reference/config_files/profiles.html), an example can be found [here](.github/conan/profiles/windows-clang-cl)
- Build (from x64 Native Tools Command Prompt for VS2022)

```console
conan install . --build=missing --profile=clang-release
cmake --preset conan-clang-release -DCMAKE_PREFIX_PATH=C:\\Qt\\6.8.1\\msvc2022_64 -DPA_SUBMIT_URL=<SUBMIT_URL> -DPA_LOOKUP_URL=<LOOKUP_URL>
cmake --build --preset conan-clang-release
```

### Linux

#### Requirements

- [conan](https://conan.io/) >= 2.0
- [Qt](https://www.qt.io/) >= 6.6.0
- [gcc](https://clang.llvm.org/) >= 12
- [ninja](https://ninja-build.org/) >= 1.10.2
- [cmake](https://cmake.org/) >= 3.17
- [python](https://www.python.org/) >= 3.7
- [rust](https://www.rust-lang.org/) >= 1.60

#### Steps

- [Setup a conan profile for gcc](https://docs.conan.io/2/reference/config_files/profiles.html), an example can be found [here](.github/conan/profiles/linux-gcc)
- Build
```console
conan install . --build=missing --profile=gcc-release
cmake --preset conan-gcc-release -DPA_SUBMIT_URL=<SUBMIT_URL> -DPA_LOOKUP_URL=<LOOKUP_URL>
cmake --build --preset conan-gcc-release
```

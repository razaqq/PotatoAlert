# PotatoAlert

## Screenshots

![default](https://i.imgur.com/DEX3wjo.png)

## Warning
This tool is by no means meant for stat shaming or being toxic towards other players in general.
Please behave yourself in chat.
If you think you cannot follow this simple rule, then you hereby don't have my permission to use this tool.

## Use pre-compiled build (recommended)
You can find pre-compiled builds for Windows and Linux [here](https://github.com/razaqq/PotatoAlert/releases).

1. Just start the binary, there is no install.
2. Go [here](https://developers.wargaming.net/applications/) and create an application for WoWs. Enter the API key in the settings.
3. Set your replays folder in the settings.


## Run with python without compiling
Otherwise you can run it directly with python 3.7.
For that you will need to install few dependencies:

```console
python -m pip install -r requirements.txt
```

## Compile yourself
Get dependencies like [here](#Run-with-python-without-compiling).
Now you have two options: Use a GUI to compile or not.

No GUI:
1. Get PyInstaller
2. Open build.py and edit lib paths
3. Run it to compile

With GUI:
1. Get https://pypi.org/project/auto-py-to-exe/
2. Add --paths "PYTHON/Lib/site - packages/PyQt5/Qt/bin" manual entry, replace PYTHON with the path to your python install
3. Add /assets as additional files
4. Compile

In both cases you will find your binary in dist/
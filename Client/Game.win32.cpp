// Copyright 2021 <github.com/razaqq>

#include "Game.hpp"

#include "win32.h"

#include <optional>
#include <filesystem>


namespace fs = std::filesystem;

std::optional<fs::path> PotatoAlert::Game::GetGamePath()
{
	HKEY key;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WOWS.WW.PRODUCTION", 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		unsigned char buffer[MAX_PATH];
		DWORD bufferSize = sizeof(buffer);
		if (RegQueryValueExA(key, "InstallLocation", NULL, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
		{
			fs::path gamePath(reinterpret_cast<char*>(buffer));
			if (!gamePath.empty() && fs::exists(gamePath))
			{
				return gamePath;
			}
		}
	}
	return {};
}

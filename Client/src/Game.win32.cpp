// Copyright 2021 <github.com/razaqq>

#include "Client/Game.hpp"

#include "win32.h"

#include <optional>
#include <filesystem>


namespace fs = std::filesystem;

std::optional<std::string> PotatoAlert::Client::Game::GetGamePath()
{
	HKEY key;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WOWS.WW.PRODUCTION)", 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		unsigned char buffer[MAX_PATH];
		DWORD bufferSize = sizeof(buffer);
		if (RegQueryValueExA(key, "InstallLocation", nullptr, nullptr, buffer, &bufferSize) == ERROR_SUCCESS)
		{
			const char* gamePathStr = reinterpret_cast<char*>(buffer);
			const fs::path gamePath(gamePathStr);
			if (fs::exists(gamePath) && !gamePath.empty())
			{
				return gamePathStr;
			}
		}
	}
	return {};
}

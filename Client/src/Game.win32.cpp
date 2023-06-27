// Copyright 2021 <github.com/razaqq>

#include "Client/Game.hpp"

#include "win32.h"

#include <optional>
#include <filesystem>


namespace fs = std::filesystem;

std::optional<std::string> PotatoAlert::Client::Game::GetGamePath()
{
	HKEY hKey;

	if (RegOpenKeyExA(HKEY_CURRENT_USER, R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall)", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return {};
	}

	CHAR achKey[255];
	DWORD cbName;
	DWORD cSubKeys = 0;
	FILETIME ftLastWriteTime;

	if (RegQueryInfoKeyA(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, &ftLastWriteTime) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return {};
	}

	for (DWORD i = 0; i < cSubKeys; i++)
	{
		if (RegEnumKeyExA(hKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime) == ERROR_SUCCESS)
		{
			HKEY subKey;
			if (RegOpenKeyExA(hKey, achKey, 0, KEY_READ, &subKey) == ERROR_SUCCESS)
			{
				DWORD displayNameSize = MAX_PATH;
				CHAR displayName[MAX_PATH];
				if (RegQueryValueExA(subKey, "DisplayName", nullptr, nullptr, (LPBYTE)displayName, &displayNameSize) != ERROR_SUCCESS)
				{
					RegCloseKey(subKey);
					continue;
				}

				DWORD publisherSize = MAX_PATH;
				CHAR publisher[MAX_PATH];
				if (RegQueryValueExA(subKey, "Publisher", nullptr, nullptr, (LPBYTE)publisher, &publisherSize) != ERROR_SUCCESS)
				{
					RegCloseKey(subKey);
					continue;
				}

				if (strcmp(publisher, "Wargaming.net") == 0 && strcmp(displayName, "World_of_Warships") == 0)
				{
					DWORD installLocationSize = MAX_PATH;
					CHAR installLocation[MAX_PATH];
					if (RegQueryValueExA(subKey, "InstallLocation", nullptr, nullptr, (LPBYTE)installLocation, &installLocationSize) == ERROR_SUCCESS)
					{
						RegCloseKey(hKey);
						RegCloseKey(subKey);
						const fs::path gamePath(installLocation);
						if (fs::exists(gamePath) && !gamePath.empty())
						{
							return installLocation;
						}
						return std::string(installLocation);
					}
					else
					{
						RegCloseKey(hKey);
						RegCloseKey(subKey);
						return {};
					}
				}
			}
			RegCloseKey(subKey);
		}
	}
	RegCloseKey(hKey);
	return {};
}

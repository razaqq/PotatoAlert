// Copyright 2021 <github.com/razaqq>

#include "Client/Game.hpp"

#include "Core/Defer.hpp"
#include "Core/Result.hpp"

#include "win32.h"

#include <optional>
#include <filesystem>


using PotatoAlert::Core::MakeDefer;
using PotatoAlert::Core::Result;

namespace fs = std::filesystem;

std::optional<fs::path> PotatoAlert::Client::Game::GetGamePath()
{
	HKEY hKey;
	auto defer = MakeDefer([&hKey]
	{
		RegCloseKey(hKey);
	});

	if (RegOpenKeyExW(HKEY_CURRENT_USER, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall)", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return {};
	}

	DWORD cSubKeys = 0;
	DWORD cbMaxSubKeyLen = 0;

	if (RegQueryInfoKeyW(hKey, nullptr, nullptr, nullptr, &cSubKeys, &cbMaxSubKeyLen, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
	{
		return {};
	}

	WCHAR subKeyName[cbMaxSubKeyLen + 1];
	DWORD subKeyNameLength = cbMaxSubKeyLen + 1;

	for (DWORD i = 0; i < cSubKeys; i++)
	{
		if (RegEnumKeyExW(hKey, i, subKeyName, &subKeyNameLength, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
		{
			HKEY subKey;
			auto defer2 = MakeDefer([&subKey]
			{
				RegCloseKey(subKey);
			});

			if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &subKey) == ERROR_SUCCESS)
			{
				DWORD cbDataDisplayName;
				if (RegQueryValueExW(subKey, L"DisplayName", nullptr, nullptr, nullptr, &cbDataDisplayName) != ERROR_SUCCESS)
				{
					continue;
				}

				WCHAR displayName[cbDataDisplayName];
				if (RegQueryValueExW(subKey, L"DisplayName", nullptr, nullptr, (LPBYTE)displayName, &cbDataDisplayName) != ERROR_SUCCESS)
				{
					continue;
				}

				DWORD cbDataPublisherName;
				if (RegQueryValueExW(subKey, L"Publisher", nullptr, nullptr, nullptr, &cbDataPublisherName) != ERROR_SUCCESS)
				{
					continue;
				}

				WCHAR publisher[cbDataPublisherName];
				if (RegQueryValueExW(subKey, L"Publisher", nullptr, nullptr, (LPBYTE)publisher, &cbDataPublisherName) != ERROR_SUCCESS)
				{
					continue;
				}

				if (wcscmp(publisher, L"Wargaming.net") == 0 && wcscmp(displayName, L"World_of_Warships") == 0)
				{
					DWORD installLocationSize = MAX_PATH;
					WCHAR installLocation[MAX_PATH];
					if (RegQueryValueExW(subKey, L"InstallLocation", nullptr, nullptr, (LPBYTE)installLocation, &installLocationSize) == ERROR_SUCCESS)
					{
						fs::path gamePath(installLocation);
						if (fs::exists(gamePath) && !gamePath.empty())
						{
							return std::move(gamePath);
						}
						return {};
					}
					return {};
				}
			}
		}
	}
	return {};
}

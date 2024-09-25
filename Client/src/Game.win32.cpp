// Copyright 2021 <github.com/razaqq>

#include "Client/Game.hpp"

#include "Core/Defer.hpp"
#include "Core/String.hpp"
#include "Core/Result.hpp"

#include "win32.h"

#include <filesystem>
#include <optional>
#include <string>


using PotatoAlert::Core::Result;
using PotatoAlert::Core::String::TrimExtraNulls;

namespace fs = std::filesystem;

namespace {

static std::optional<std::wstring> GetRegistryString(HKEY key, std::wstring_view valueName, DWORD sizeBytes = 0)
{
	if (sizeBytes == 0)
	{
		if (RegQueryValueExW(key, valueName.data(), nullptr, nullptr, nullptr, &sizeBytes) != ERROR_SUCCESS)
		{
			return {};
		}
	}

	std::wstring lpData;
	lpData.resize(sizeBytes / sizeof(WCHAR));
	if (RegQueryValueExW(key, valueName.data(), nullptr, nullptr, (LPBYTE)lpData.data(), &sizeBytes) != ERROR_SUCCESS)
	{
		return {};
	}

	TrimExtraNulls(lpData);
	return lpData;
}

}

std::vector<fs::path> PotatoAlert::Client::Game::GetDefaultGamePaths()
{
	HKEY hKey;
	PA_DEFER
	{
		RegCloseKey(hKey);
	};

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

	std::wstring subKeyName;
	subKeyName.resize(cbMaxSubKeyLen + 1);
	const DWORD subKeyNameLength = cbMaxSubKeyLen + 1;

	std::vector<fs::path> out;

	for (DWORD i = 0; i < cSubKeys; i++)
	{
		DWORD cchName = subKeyNameLength;
		if (RegEnumKeyExW(hKey, i, subKeyName.data(), &cchName, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
		{
			HKEY subKey;
			PA_DEFER
			{
				RegCloseKey(subKey);
			};

			if (RegOpenKeyExW(hKey, subKeyName.data(), 0, KEY_READ, &subKey) == ERROR_SUCCESS)
			{
				std::optional displayName = GetRegistryString(subKey, L"DisplayName");
				if (!displayName)
				{
					continue;
				}

				std::optional publisher = GetRegistryString(subKey, L"Publisher");
				if (!publisher)
				{
					continue;
				}

				const bool game = Core::String::StartsWith<wchar_t>(*displayName, L"World_of_Warships");

				if (*publisher == L"Wargaming.net" && game)
				{
					if (std::optional installLocation = GetRegistryString(subKey, L"InstallLocation"))
					{
						fs::path gamePath(*installLocation);
						if (fs::exists(gamePath) && !gamePath.empty())
						{
							out.emplace_back(std::move(gamePath));
						}
					}
				}
			}
		}
	}
	return out;
}

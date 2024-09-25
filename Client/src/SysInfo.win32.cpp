// Copyright 2024 <github.com/razaqq>

#include "Client/SysInfo.hpp"

#include "Core/Defer.hpp"
#include "Core/Result.hpp"
#include "Core/String.hpp"

#include "win32.h"
#include "VersionHelpers.h"

#include <optional>
#include <string>
#include <system_error>


using PotatoAlert::Client::SysInfo;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::String::TrimExtraNulls;

namespace {

#if 0
typedef LONG NTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW GetRealOSVersion()
{
	if (HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll"))
	{
		if (RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion"))
		{
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if (STATUS_SUCCESS == fxPtr(&rovi))
			{
				return rovi;
			}
		}
	}
	RTL_OSVERSIONINFOW rovi = { 0 };
	return rovi;
}
#endif

static std::optional<std::string> GetRegistryString(HKEY key, std::string_view valueName, DWORD sizeBytes = 0)
{
	if (sizeBytes == 0)
	{
		if (RegQueryValueExA(key, valueName.data(), nullptr, nullptr, nullptr, &sizeBytes) != ERROR_SUCCESS)
		{
			return {};
		}
	}

	std::string lpData;
	lpData.resize(sizeBytes / sizeof(CHAR));
	if (RegQueryValueExA(key, valueName.data(), nullptr, nullptr, (LPBYTE)lpData.data(), &sizeBytes) != ERROR_SUCCESS)
	{
		return {};
	}

	TrimExtraNulls(lpData);
	return lpData;
}

}


Result<SysInfo> PotatoAlert::Client::GetSysInfo()
{
	HANDLE hProcess = GetCurrentProcess();
	USHORT processMachine;
	USHORT nativeMachine;
	if (IS_ERROR(IsWow64Process2(hProcess, &processMachine, &nativeMachine)))
	{
		return PA_ERROR(std::error_code((int)GetLastError(), std::system_category()));
	}

	HKEY hKey;
	PA_DEFER
	{
		RegCloseKey(hKey);
	};
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, LR"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return PA_ERROR(std::error_code((int)GetLastError(), std::system_category()));
	}
	std::optional productName = GetRegistryString(hKey, "ProductName");
	if (!productName)
	{
		return PA_ERROR(std::error_code((int)GetLastError(), std::system_category()));
	}

	SYSTEM_INFO sysInfo;
	GetNativeSystemInfo(&sysInfo);

	ProcessorArchitecture arch = ProcessorArchitecture::Unknown;
	switch (sysInfo.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64:
			arch = ProcessorArchitecture::Amd64;
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			arch = ProcessorArchitecture::Arm;
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			arch = ProcessorArchitecture::IA64;
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			arch = ProcessorArchitecture::X86;
			break;
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
			arch = ProcessorArchitecture::Unknown;
			break;
	}

	return SysInfo
	{
		.Os = OsType::Windows,
		.Arch = arch,
		.Release = *productName,
	};
}

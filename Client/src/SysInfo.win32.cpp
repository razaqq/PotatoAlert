// Copyright 2024 <github.com/razaqq>

#include "Client/SysInfo.hpp"

#include "Core/Defer.hpp"
#include "Core/Result.hpp"
#include "Core/String.hpp"

//#define WIN32_USER
//#include "win32.h"
//#include <Windows.h>
#include <atlbase.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include <optional>
#include <string>
#include <system_error>


using PotatoAlert::Client::SysInfo;
using PotatoAlert::Core::Result;
using PotatoAlert::Core::String::TrimExtraNulls;

namespace {

#define CheckHRes(hRes)                                                 \
	if (FAILED(hRes))                                                   \
	{                                                                   \
		return PA_ERROR(std::error_code(hRes, std::system_category())); \
	}                                                                   \
	void

Result<std::string> WmiGetOsCaption()
{
	HRESULT hRes = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (hRes != S_OK && hRes != S_FALSE)
	{
		return PA_ERROR(std::error_code(hRes, std::system_category()));
	}
	hRes = ::CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
								  RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
								  nullptr, EOAC_NONE, nullptr);
	CheckHRes(hRes);

	CComPtr<IWbemLocator> pLocator;
	hRes = pLocator.CoCreateInstance(CLSID_WbemLocator);
	CheckHRes(hRes);

	CComPtr<IWbemServices> pService;
	hRes = pLocator->ConnectServer(CComBSTR(L"root\\cimv2"), nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT,
								   nullptr, nullptr, &pService);
	CheckHRes(hRes);


	CComPtr<IEnumWbemClassObject> pEnum;
	hRes = pService->ExecQuery(CComBSTR(L"WQL"), CComBSTR(L"Select Caption from Win32_OperatingSystem"), WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnum);
	CheckHRes(hRes);

	ULONG uObjectCount = 0;
	CComPtr<IWbemClassObject> pWmiObject;
	hRes = pEnum->Next(WBEM_INFINITE, 1, &pWmiObject, &uObjectCount);
	CheckHRes(hRes);

	CComVariant cvtCaption;
	hRes = pWmiObject->Get(L"Caption", 0, &cvtCaption, nullptr, nullptr);
	CheckHRes(hRes);

	CoUninitialize();

	return CW2A(cvtCaption.bstrVal).m_psz;
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

	PA_TRY(productName, WmiGetOsCaption());

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
		.Release = productName,
	};
}

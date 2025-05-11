// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Result.hpp"

#include <string>


namespace PotatoAlert::Client {

enum class OsType
{
	Windows,
	Linux,
	Unknown,
};
inline constexpr std::string_view ToString(OsType os)
{
	switch (os)
	{
		using enum OsType;
		case Windows:
			return "windows";
		case Linux:
			return "linux";
		case Unknown:
			return "unknown";
	}
	return "unknown";
}

enum class ProcessorArchitecture
{
	Amd64,
	Arm,
	IA64,
	X86,
	Unknown
};
inline constexpr std::string_view ToString(ProcessorArchitecture arch)
{
	switch (arch)
	{
		using enum ProcessorArchitecture;
		case Amd64:
			return "amd64";
		case Arm:
			return "arm";
		case IA64:
			return "ia64";
		case X86:
			return "x86";
		case Unknown:
			return "unknown";
	}
	return "unknown";
}

struct SysInfo
{
	OsType Os;
	ProcessorArchitecture Arch;
	std::string Release;
};

Core::Result<SysInfo> GetSysInfo();

}  // namespace PotatoAlert::Client

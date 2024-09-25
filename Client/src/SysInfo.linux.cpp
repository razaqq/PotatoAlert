// Copyright 2024 <github.com/razaqq>

#include "Client/SysInfo.hpp"

#include "Core/Result.hpp"

#include <sys/utsname.h>

#include <string>
#include <system_error>


using PotatoAlert::Client::SysInfo;
using PotatoAlert::Core::Result;

Result<SysInfo> PotatoAlert::Client::GetSysInfo()
{
	utsname uts;
	int ret = uname(&uts);
	if (ret != 0)
	{
		return PA_ERROR(std::error_code(errno, std::system_category()));
	}
	std::string_view release = uts.release;
	std::string_view machine = uts.machine;

	ProcessorArchitecture arch = ProcessorArchitecture::Unknown;
	// https://wiki.debian.org/ArchitectureSpecificsMemo
	if (machine.starts_with("x86_64"))
	{
		arch = ProcessorArchitecture::Amd64;
	}
	else if (machine.starts_with("i386") || machine.starts_with("i486") || machine.starts_with("i586") || machine.starts_with("i686"))
	{
		arch = ProcessorArchitecture::X86;
	}
	else if (machine.starts_with("arm"))
	{
		arch = ProcessorArchitecture::Arm;
	}
	else if (machine == "ia64")
	{
		arch = ProcessorArchitecture::IA64;
	}

	return SysInfo
	{
		.Os = OsType::Linux,
		.Arch = arch,
		.Release = std::string(release),
	};
}

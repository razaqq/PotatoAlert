// Copyright 2021 <github.com/razaqq>

#include "Core/Process.hpp"
#include "Core/String.hpp"

#include <spawn.h>
#include <stdlib.h>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>


namespace c = PotatoAlert::Core;

void c::ExitCurrentProcess(uint32_t code)
{
	exit(code);
}

void PotatoAlert::Core::ExitCurrentProcessWithError(uint32_t code)
{
	std::cout << "A critical error has occurred, please view the logs and report the error to the developer. Exit Code " << code << std::endl;
	ExitCurrentProcess(code);
}

bool c::CreateNewProcess(const std::filesystem::path& path, std::string_view args, bool elevated)
{
	pid_t pid;
	const std::string pathStr = path.string();
	std::vector<std::string> argsSplit = Core::String::Split(args, " ");
	argsSplit.insert(argsSplit.begin(), pathStr);
	auto argsPtrs = argsSplit | std::views::transform([](const std::string& s)
	{
		return const_cast<char*>(s.c_str());
	});
	const std::vector<char*> argsPtr(argsPtrs.begin(), argsPtrs.end());
	return posix_spawn(&pid, pathStr.c_str(), nullptr, nullptr, argsPtr.data(), nullptr) == 0;
}

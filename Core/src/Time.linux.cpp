// Copyright 2021 <github.com/razaqq>

#include "Core/Time.hpp"

#include <ctime>
#include <string>


std::string PotatoAlert::Core::Time::GetTimeStamp(std::string_view fmt)
{
	std::tm bt{};
	auto now = std::time(nullptr);
	localtime_r(&now, &bt);
	char buf[64];
	return {buf, std::strftime(buf, sizeof(buf), fmt.data(), &bt)};
}

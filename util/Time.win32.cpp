// Copyright 2021 <github.com/razaqq>

#include "Time.hpp"
#include <algorithm>
#include <ctime>
#include <mutex>
#include <string>


std::string PotatoAlert::Time::GetTimeStamp(std::string_view fmt)
{
	std::tm bt {};
	auto now = std::time(nullptr);
	localtime_s(&bt, &now);
	char buf[64];
	return {buf, std::strftime(buf, sizeof(buf), fmt.data(), &bt)};
}
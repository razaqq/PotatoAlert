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
	localtime_r(&now, &bt);
	char buf[64];
	std::string out = {buf, std::strftime(buf, sizeof(buf), fmt.data(), &bt)};
	std::replace(out.begin(), out.end(), ':', '-');
	return out;
}

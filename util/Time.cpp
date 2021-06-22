// Copyright 2021 <github.com/razaqq>

#include "Time.hpp"
#include <algorithm>
#include <ctime>
#include <mutex>
#include <string>


inline std::tm localtime_xp(std::time_t timer)
{
	std::tm bt {};
#if defined(__unix__)
	localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
	localtime_s(&bt, &timer);
#else
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	bt = *std::localtime(&timer);
#endif
	return bt;
}

std::string PotatoAlert::Time::GetTimeStamp(std::string_view fmt)
{
	auto bt = localtime_xp(std::time(nullptr));
	char buf[64];
	std::string out = {buf, std::strftime(buf, sizeof(buf), fmt.data(), &bt)};
	std::replace(out.begin(), out.end(), ':', '-');
	return out;
}

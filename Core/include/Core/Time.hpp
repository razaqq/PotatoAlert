// Copyright 2021 <github.com/razaqq>
#pragma once

#include <chrono>
#include <optional>
#include <string>


namespace PotatoAlert::Core::Time {

using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>;
using TimePointLocal = std::chrono::time_point<std::chrono::local_t, std::chrono::seconds>;

std::string GetTimeStamp(std::string_view fmt = "%F_%T");

#ifdef PA_DATE_LOCALTIME
TimePointLocal NowLocal();
#endif

std::string TimeToStr(TimePoint time, std::string_view fmt = "{:%H:%M:%S}");

std::optional<TimePoint> StrToTime(const std::string& d, std::string_view fmt = "%Y-%m-%d");

}  // namespace PotatoAlert::Time

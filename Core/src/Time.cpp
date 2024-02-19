#include "Core/Format.hpp"
#include "Core/Time.hpp"

#include <date/date.h>
#include <date/tz.h>

#include <chrono>
#include <optional>
#include <string>
#include <sstream>


using PotatoAlert::Core::Time::TimePoint;
using PotatoAlert::Core::Time::TimePointLocal;

#ifdef PA_DATE_LOCALTIME
// This needs linking to date-tz
TimePointLocal PotatoAlert::Core::Time::NowLocal()
{
	using Clock = std::chrono::system_clock;
	const date::zoned_time<Clock::duration> now = date::zoned_time(date::current_zone(), Clock::now());
	return TimePointLocal(std::chrono::time_point_cast<std::chrono::seconds>(now.get_local_time()).time_since_epoch());
}
#endif

std::string PotatoAlert::Core::Time::TimeToStr(TimePoint time, std::string_view fmt)
{
	return fmt::format(fmt::runtime(fmt), time);
}

std::optional<TimePoint> PotatoAlert::Core::Time::StrToTime(const std::string& d, std::string_view fmt)
{
	TimePoint tp;
	std::stringstream ss(d);
	if (date::from_stream(ss, fmt.data(), tp))
	{
		return tp;
	}
	return {};
}

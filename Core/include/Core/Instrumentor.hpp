// Copyright 2021 <github.com/razaqq>
#pragma once

#include "AsciiTable.hpp"
#include "Log.hpp"
#include "Singleton.hpp"
#include "String.hpp"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>


namespace core = PotatoAlert::Core;

namespace PotatoAlert::Core {

using Micros = std::chrono::duration<double, std::micro>;
using MicrosRep = std::chrono::microseconds::rep;

struct ProfileResult
{
	std::string_view name;
	Micros startTime;
	std::chrono::microseconds elapsedTime;
	std::thread::id threadId;
};

class Instrumentor
{
public:
	PA_SINGLETON(Instrumentor);

	void WriteResult(const ProfileResult& result)
	{
		std::lock_guard lock(m_mutex);

		LOG_INFO("Name: {}, TID: {}, Start: {}, Duration: {}", 
			result.name, result.threadId, result.startTime.count(), result.elapsedTime.count());

		if (m_timings.contains(result.name.data()))
		 	m_timings[result.name.data()].push_back(result.elapsedTime.count());
		m_timings.insert({result.name.data(), std::vector{ result.elapsedTime.count() }});
	}

private:
	Instrumentor() = default;
	~Instrumentor()
	{
		AsciiTable<std::string, MicrosRep, MicrosRep, MicrosRep, MicrosRep, size_t> table({ "Name", "Average", "Min", "Max", "Total", "Runs" });

		for (auto& [name, timings] : m_timings)
		{
			MicrosRep total = std::accumulate(timings.begin(), timings.end(), MicrosRep{ 0 });
			MicrosRep avg = total / static_cast<MicrosRep>(timings.size());
			table.AddRow(name, avg, *std::min(timings.begin(), timings.end()), *std::max(timings.begin(), timings.end()), total, timings.size());
		}

		table.SortByColumn<1>(SortOrder::Descending);

		std::stringstream stream;
		table.Print(stream);
		LOG_INFO("\n{}", stream.str());
	}
	std::mutex m_mutex;
	std::unordered_map<std::string, std::vector<MicrosRep>> m_timings;
};

class Timer
{
public:
	Timer() = delete;
	explicit Timer(std::string_view name) : m_name(name), m_running(true), m_startTime(std::chrono::steady_clock::now()) {}
	Timer(const Timer&) = delete;
	Timer(Timer&&) = delete;
	Timer& operator=(Timer&) = delete;
	Timer& operator=(Timer&&) = delete;

	[[nodiscard]] MicrosRep Elapsed() const
	{
		if (m_running)
		{
			return (std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()).time_since_epoch() -
					std::chrono::time_point_cast<std::chrono::microseconds>(m_startTime).time_since_epoch()).count();
		}
		return 0;
	}

	~Timer()
	{
		if (m_running)
		{
			auto endTime = std::chrono::steady_clock::now();

			auto elapsed = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch() -
				std::chrono::time_point_cast<std::chrono::microseconds>(m_startTime).time_since_epoch();

			m_running = false;

			Instrumentor::Instance().WriteResult({ m_name, Micros{ m_startTime.time_since_epoch() }, elapsed, std::this_thread::get_id() });
		}
	}
private:
	std::string m_name;
	bool m_running;
	std::chrono::time_point<std::chrono::steady_clock> m_startTime;
};

}  // namespace PotatoAlert::Core

#ifdef PA_PROFILE

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define PA_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define PA_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define PA_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define PA_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define PA_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define PA_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define PA_FUNC_SIG __func__
#else
#define PA_FUNC_SIG "PA_FUNC_SIG unknown!"
#endif

#define PA_PROFILE_SCOPE_LINE(name, line) ::core::Timer timer##line(::core::String::ReplaceAll(::core::String::ReplaceAll(name, "__cdecl ", ""), "PotatoAlert::", ""))
#define PA_PROFILE_SCOPE(name) PA_PROFILE_SCOPE_LINE(name, __LINE__)
#define PA_PROFILE_FUNCTION() PA_PROFILE_SCOPE(PA_FUNC_SIG)

#else
#define PA_PROFILE_SCOPE_LINE(name, line)
#define PA_PROFILE_SCOPE(name)
#define PA_PROFILE_FUNCTION()
#endif

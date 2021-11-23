// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Log.hpp"
#include "String.hpp"

#include <chrono>
#include <string>
#include <thread>


namespace PotatoAlert {

using Micros = std::chrono::duration<double, std::micro>;

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

	Instrumentor(const Instrumentor&) = delete;
	Instrumentor(Instrumentor&&) = delete;
	Instrumentor& operator=(Instrumentor&) = delete;
	Instrumentor& operator=(Instrumentor&&) = delete;

	static Instrumentor& Instance()
	{
		static Instrumentor i;
		return i;
	}

	void WriteResult(const ProfileResult& result)
	{
		std::lock_guard lock(m_mutex);

		LOG_INFO("Name: {}, TID: {}, Start: {}, Duration: {}", 
			result.name, result.threadId, result.startTime.count(), result.elapsedTime.count());
	}

private:
	Instrumentor() = default;
	~Instrumentor() = default;
	std::mutex m_mutex;
};

class Timer
{
public:
	explicit Timer(std::string_view name) : m_name(name), m_running(true), m_startTime(std::chrono::steady_clock::now()) {}
	Timer(const Timer&) = delete;
	Timer(Timer&&) = delete;
	Timer& operator=(Timer&) = delete;
	Timer& operator=(Timer&&) = delete;

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

}  // namespace PotatoAlert

#ifndef NDEBUG

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

#define PA_PROFILE_SCOPE_LINE(name, line) ::PotatoAlert::Timer timer##line(String::ReplaceAll(String::ReplaceAll(name, "__cdecl ", ""), "PotatoAlert::", ""))
#define PA_PROFILE_SCOPE(name) PA_PROFILE_SCOPE_LINE(name, __LINE__)
#define PA_PROFILE_FUNCTION() PA_PROFILE_SCOPE(PA_FUNC_SIG)
#else
#define PA_PROFILE_SCOPE_LINE(name, line)
#define PA_PROFILE_SCOPE(name)
#define PA_PROFILE_FUNCTION()
#endif

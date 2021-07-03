// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Time.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <format>
#include <QString>


using namespace PotatoAlert::Time;

namespace PotatoAlert {

class Logger
{
public:
	Logger(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger& operator=(Logger&&) = delete;

	static QString GetDir();

	// DEBUG
	static void Debug(const char* text)
	{
	#ifndef NDEBUG
		std::cout << Format(Level::Debug, text) << std::endl;
	#endif
	}
	static void Debug(const std::string& text)
	{
	#ifndef NDEBUG
		std::cout << Format(Level::Debug, text) << std::endl;
	#endif
	}
	template<typename... TArgs>
	static void Debug(const char* format, TArgs&&... args)
	{
	#ifndef NDEBUG
		std::cout << Format(Level::Debug, format, args...) << std::endl;
	#endif
	}

	// INFO
	static void Info(const char* text) { Instance().Write(Level::Info, text); }
	static void Info(const std::string& text) { Instance().Write(Level::Info, text); }
	template<typename... TArgs>
	static void Info(const char* format, TArgs&&... args) { Instance().Write(Level::Info, format, args...); }

	// WARN
	static void Warn(const char* text) { Instance().Write(Level::Warn, text); }
	static void Warn(const std::string& text) { Instance().Write(Level::Warn, text); }
	template<typename... TArgs>
	static void Warn(const char* format, TArgs&&... args) { Instance().Write(Level::Warn, format, args...); }

	// ERROR
	static void Error(const char* text) { Instance().Write(Level::Error, text); }
	static void Error(const std::string& text) { Instance().Write(Level::Error, text); }
	template<typename... TArgs>
	static void Error(const char* format, TArgs&&... args) { Instance().Write(Level::Error, format, args...); }
private:
	Logger();
	~Logger();

	static Logger& Instance()
	{
		static Logger l;
		return l;
	}

	enum class Level
	{
		Debug,
		Info,
		Warn,
		Error
	};

	template<typename... TArgs>
	static std::string Format(Level level, const char* format, TArgs&&... args)
	{
		return std::format("{} - [{:5}] {}", GetTimeStamp(m_timeFormat), GetPrefix(level), std::format(format, args...));
	}
	static std::string Format(Level level, const char* text)
	{
		return std::format("{} - [{:5}] {}", GetTimeStamp(m_timeFormat), GetPrefix(level), text);
	}
	static std::string Format(Level level, const std::string& text)
	{
		return std::format("{} - [{:5}] {}", GetTimeStamp(m_timeFormat), GetPrefix(level), text);
	}

	static constexpr std::string_view m_timeFormat = "%d-%m-%Y %H:%M:%S";
	static constexpr std::string_view GetPrefix(Level level)
	{
		switch (level)
		{
			case Level::Debug: return "DEBUG";
			case Level::Info: return "INFO";
			case Level::Warn: return "WARN";
			case Level::Error: return "ERROR";
		}
	}
	static constexpr std::ostream& GetOutput(Level level)
	{
		switch (level)
		{
			case Level::Debug: case Level::Info: case Level::Warn: return std::cout;
			case Level::Error: return std::cerr;
		}
	}

	void Write(Level level, const std::string& text)
	{
		auto formatted = Format(level, text);
		GetOutput(level) << formatted << std::endl;
		this->_ofs << formatted << std::endl;
	}
	void Write(Level level, const char* text)
	{
		auto formatted = Format(level, text);
		GetOutput(level) << formatted << std::endl;
		this->_ofs << formatted << std::endl;
	}
	template<typename... TArgs>
	void Write(Level level, const char* format, TArgs&&... args)
	{
		auto formatted = Format(level, format, args...);
		GetOutput(level) << formatted << std::endl;
		this->_ofs << formatted << std::endl;
	}

	std::ofstream _ofs;  // TODO
};

}  // namespace PotatoAlert

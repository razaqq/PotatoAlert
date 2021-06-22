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
	Logger& operator= (const Logger) = delete;

	static QString GetDir();

	// DEBUG
	static void Debug(const char* text);
	static void Debug(const std::string& text);

	template<typename... TArgs>
	static void Debug(const char* format, TArgs&&... args)
	{
	#ifndef NDEBUG
			auto text = std::format(format, args...);
			std::cout << GetTimeStamp(timeFormat) << debugPrefix << text << std::endl;
	#endif
	}

	// INFO
	static void Info(const char* text) { return Logger::Get().IInfo(text); }
	static void Info(const std::string& text) { return Logger::Get().IInfo(text); }
	template<typename... TArgs>
	static void Info(const char* format, TArgs&&... args) { return Logger::Get().template IInfo(format, args...); }

	// WARN
	static void Warn(const char* text) { return Logger::Get().IWarn(text); }
	static void Warn(const std::string& text) { return Logger::Get().IWarn(text); }
	template<typename... TArgs>
	static void Warn(const char* format, TArgs&&... args) { return Logger::Get().template IWarn(format, args...); }

	// ERROR
	static void Error(const char* text) { return Logger::Get().IError(text); }
	static void Error(const std::string& text) { return Logger::Get().IError(text); }
	template<typename... TArgs>
	static void Error(const char* format, TArgs&&... args) { return Logger::Get().template IError(format, args...); }
private:
	Logger();
	~Logger();

	static constexpr std::string_view timeFormat = "%d-%m-%Y %H:%M:%S";
	static constexpr std::string_view debugPrefix = " - [DEBUG] ";
	static constexpr std::string_view infoPrefix = " - [INFO] ";
	static constexpr std::string_view warnPrefix = " - [WARN] ";
	static constexpr std::string_view errorPrefix = " - [ERROR] ";

	static Logger& Get()
	{
		static Logger l;
		return l;
	}

	// INFO
	void IInfo(const char* text);
	void IInfo(const std::string& text);

	template<typename... TArgs>
	void IInfo(const char* format, TArgs&&... args)
	{
		auto text = std::format(format, args...);
		std::cout << GetTimeStamp(timeFormat) << infoPrefix << text << std::endl;
		this->_ofs << GetTimeStamp(timeFormat) << infoPrefix << text << std::endl;

	}

	// WARN
	void IWarn(const char* text);
	void IWarn(const std::string& text);

	template<typename... TArgs>
	void IWarn(const char* format, TArgs&&... args)
	{
		auto text = std::format(format, args...);
		std::cout << GetTimeStamp(timeFormat) << warnPrefix << text << std::endl;
		this->_ofs << GetTimeStamp(timeFormat) << warnPrefix << text << std::endl;
	}

	// ERROR
	void IError(const char* text);
	void IError(const std::string& text);

	template<typename... TArgs>
	void IError(const char* format, TArgs&&... args)
	{
		auto text = std::format(format, args...);
		std::cerr << GetTimeStamp(timeFormat) << errorPrefix << text << std::endl;
		this->_ofs << GetTimeStamp(timeFormat) << errorPrefix << text << std::endl;
	}

	std::ofstream _ofs;
};

}  // namespace PotatoAlert

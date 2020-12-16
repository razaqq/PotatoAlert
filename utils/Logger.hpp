// Copyright 2020 <github.com/razaqq>
#pragma once

#include <fmt/format.h>
#include <string>
#include <fstream>
#include <iostream>


namespace PotatoAlert {

const char debugPrefix[] = " - [DEBUG] ";
const char infoPrefix[] = " - [INFO] ";
const char warnPrefix[] = " - [WARN] ";
const char errorPrefix[] = " - [ERROR] ";

class Logger
{
public:
	Logger(const Logger&) = delete;
	Logger& operator= (const Logger) = delete;

	// DEBUG
	static void Debug(const char* text);
	static void Debug(const std::string& text);

	template<typename... TArgs>
	static void Debug(const char* format, TArgs&&... args)
	{
	#ifndef NDEBUG
			auto text = fmt::format(format, args...);
			std::cout << getTimeString() << debugPrefix << text << std::endl;
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
		auto text = fmt::format(format, args...);
		std::cout << getTimeString() << infoPrefix << text << std::endl;
		this->_ofs << getTimeString() << infoPrefix << text << std::endl;

	}

	// WARN
	void IWarn(const char* text);
	void IWarn(const std::string& text);

	template<typename... TArgs>
	void IWarn(const char* format, TArgs&&... args)
	{
		auto text = fmt::format(format, args...);
		std::cout << getTimeString() << warnPrefix << text << std::endl;
		this->_ofs << getTimeString() << warnPrefix << text << std::endl;
	}

	// ERROR
	void IError(const char* text);
	void IError(const std::string& text);

	template<typename... TArgs>
	void IError(const char* format, TArgs&&... args)
	{
		auto text = fmt::format(format, args...);
		std::cerr << getTimeString() << errorPrefix << text << std::endl;
		this->_ofs << getTimeString() << errorPrefix << text << std::endl;
	}

	std::ofstream _ofs;
	static std::string getTimeString();
};

} // namespace PotatoAlert

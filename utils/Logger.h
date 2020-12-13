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
	Logger();
	~Logger();

	static void Debug(const char* text);
	static void Debug(const std::string& text);

	template<typename... TArgs>
	static void Debug(const char* format, TArgs&&... args)
	{
		#ifndef NDEBUG
			auto text = fmt::format(format, args...);
			std::cout << Logger::getTimeString() << debugPrefix << text << std::endl;
		#endif
	}

	void Info(const char* text);
	void Info(const std::string& text);

	template<typename... TArgs>
	void Info(const char* format, TArgs&&... args)
	{
		auto text = fmt::format(format, args...);
		std::cout << Logger::getTimeString() << infoPrefix << text << std::endl;
		this->_ofs << Logger::getTimeString() << infoPrefix << text << std::endl;

	}

	void Warn(const char* text);
	void Warn(const std::string& text);

	template<typename... TArgs>
	void Warn(const char* format, TArgs&&... args)
	{
		auto text = fmt::format(format, args...);
		std::cout << Logger::getTimeString() << warnPrefix << text << std::endl;
		this->_ofs << Logger::getTimeString() << warnPrefix << text << std::endl;
	}

	void Error(const char* text);
	void Error(const std::string& text);

	template<typename... TArgs>
	void Error(const char* format, TArgs&&... args)
	{
		auto text = fmt::format(format, args...);
		std::cerr << Logger::getTimeString() << errorPrefix << text << std::endl;
		this->_ofs << Logger::getTimeString() << errorPrefix << text << std::endl;
	}
private:
	std::ofstream _ofs;
	static std::string getTimeString() ;
	Logger(const Logger&);
	Logger& operator=(const Logger&);
};

Logger& PotatoLogger();

} // namespace PotatoAlert

// Copyright 2020 <github.com/razaqq>

#include "Logger.hpp"
#include "Time.hpp"
#include <QDir>
#include <QString>
#include <QStandardPaths>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>


using PotatoAlert::Logger;

QString Logger::GetDir()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert");
}

Logger::Logger()
{
	auto dir = Logger::GetDir();

	QDir d;
	d.mkpath(dir);
	d.setPath(dir);

	this->_ofs.open(d.filePath("PotatoAlert.log").toStdString(), std::ofstream::out | std::ofstream::ate | std::ofstream::app);
}

Logger::~Logger()
{
	if (this->_ofs.is_open())
		this->_ofs.close();
}

void Logger::Debug(const char* text)
{
	#ifndef NDEBUG
		std::cout << GetTimeStamp(timeFormat) << debugPrefix << text << std::endl;
	#endif
}

void Logger::Debug(const std::string& text)
{
#ifndef NDEBUG
	std::cout << GetTimeStamp(timeFormat) << debugPrefix << text << std::endl;
#endif
}

void Logger::IInfo(const char* text)
{
	std::cout << GetTimeStamp(timeFormat) << infoPrefix << text << std::endl;
	this->_ofs << GetTimeStamp(timeFormat) << infoPrefix << text << std::endl;
}

void Logger::IInfo(const std::string& text)
{
	std::cout << GetTimeStamp(timeFormat) << infoPrefix << text << std::endl;
	this->_ofs << GetTimeStamp(timeFormat) << infoPrefix << text << std::endl;
}

void Logger::IWarn(const char* text)
{
	std::cout << GetTimeStamp(timeFormat) << warnPrefix << text << std::endl;
	this->_ofs << GetTimeStamp(timeFormat) << warnPrefix << text << std::endl;
}

void Logger::IWarn(const std::string& text)
{
	std::cout << GetTimeStamp(timeFormat) << warnPrefix << text << std::endl;
	this->_ofs << GetTimeStamp(timeFormat) << warnPrefix << text << std::endl;
}

void Logger::IError(const char* text)
{
	std::cerr << GetTimeStamp(timeFormat) << errorPrefix << text << std::endl;
	this->_ofs << GetTimeStamp(timeFormat) << errorPrefix << text << std::endl;
}

void Logger::IError(const std::string& text)
{
	std::cerr << GetTimeStamp(timeFormat) << errorPrefix << text << std::endl;
	this->_ofs << GetTimeStamp(timeFormat) << errorPrefix << text << std::endl;
}

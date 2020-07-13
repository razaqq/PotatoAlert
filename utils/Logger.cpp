// Copyright 2020 <github.com/razaqq>

#include "Logger.h"
#include <QDir>
#include <QStandardPaths>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>


using PotatoAlert::Logger;

const char debugPrefix[] = " - [DEBUG] ";
const char infoPrefix[] = " - [INFO] ";
const char warnPrefix[] = " - [WARN] ";
const char errorPrefix[] = " - [ERROR] ";

Logger::Logger()
{
	QString dirPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

	QDir d;
	d.mkpath(dirPath);
	d.setPath(dirPath);

	_ofs.open(d.filePath("PotatoAlert.log").toStdString(), std::ofstream::out | std::ofstream::ate | std::ofstream::app);
}

Logger::~Logger()
{
	if (this->_ofs.is_open())
		this->_ofs.close();
}

void Logger::Debug(const char* text)
{
	#ifndef NDEBUG
		std::cout << Logger::getTimeString() << debugPrefix << text << std::endl;
	#endif
}

void Logger::Info(const char* text)
{
	std::cout << Logger::getTimeString() << infoPrefix << text << std::endl;
	this->_ofs << Logger::getTimeString() << infoPrefix << text << std::endl;
}

void Logger::Warn(const char* text)
{
	std::cout << Logger::getTimeString() << warnPrefix << text << std::endl;
	this->_ofs << Logger::getTimeString() << warnPrefix << text << std::endl;
}

void Logger::Error(const char* text)
{
	std::cerr << Logger::getTimeString() << errorPrefix << text << std::endl;
	this->_ofs << Logger::getTimeString() << errorPrefix << text << std::endl;
}

std::string Logger::getTimeString()
{
	std::time_t time = std::time(nullptr);

	char buffer[80];
    struct tm localTime{};
    localtime_s(&localTime, &time);
	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &localTime);

	return std::string(buffer);
}

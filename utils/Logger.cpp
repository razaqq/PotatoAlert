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
const char errorPrefix[] = " -[ERROR] ";

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
		std::cout << this->getTimeString() << debugPrefix << text << std::endl;
	#endif
}

void Logger::Info(const char* text)
{
	std::cout << this->getTimeString() << infoPrefix << text << std::endl;
	this->_ofs << this->getTimeString() << infoPrefix << text << std::endl;
}

void Logger::Warn(const char* text)
{
	std::cout << this->getTimeString() << warnPrefix << text << std::endl;
	this->_ofs << this->getTimeString() << warnPrefix << text << std::endl;
}

void Logger::Error(const char* text)
{
	std::cerr << this->getTimeString() << errorPrefix << text << std::endl;
	this->_ofs << this->getTimeString() << errorPrefix << text << std::endl;
}

std::string Logger::getTimeString() const
{
	std::time_t result = std::time(NULL);

	char buffer[80];
	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", std::localtime(&result));

	return std::string(buffer);
}

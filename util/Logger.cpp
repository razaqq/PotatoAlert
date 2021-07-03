// Copyright 2020 <github.com/razaqq>

#include "Logger.hpp"
#include "File.hpp"
#include <QDir>
#include <QString>
#include <QStandardPaths>
#include <fstream>


using PotatoAlert::Logger;

QString Logger::GetDir()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert");
}

Logger::Logger()
{
	auto dir = GetDir();

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

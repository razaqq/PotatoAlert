// Copyright 2020 <github.com/razaqq>

#include <string>
#include <initializer_list>
#include <fstream>
#include <QDir>
#include <QString>
#include <QStandardPaths>
#include "CSVWriter.hpp"
#include "Logger.hpp"


using PotatoAlert::CSVWriter;

CSVWriter::CSVWriter()
{
	this->file.open(CSVWriter::getFilePath(), std::ios::out | std::ios::app);
	if (!this->file.is_open())
		Logger::Error("Failed to open file to save matches.");
}

CSVWriter::~CSVWriter()
{
	this->file.close();
}

void CSVWriter::saveMatch(const std::string& jsonObj)
{
	if (this->file.is_open())
	{
		this->file << jsonObj + ";";
		Logger::Debug("Appending match to csv log.");
	}
}

std::string CSVWriter::getFilePath()
{
	QString dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/PotatoAlert";

	QDir d;
	d.mkpath(dirPath);
	d.setPath(dirPath);

	return d.filePath("matches.csv").toStdString();
}

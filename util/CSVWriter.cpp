// Copyright 2020 <github.com/razaqq>

#include "CSVWriter.hpp"
#include "File.hpp"
#include "Time.hpp"
#include <format>
#include <string>
#include <QDir>
#include <QString>
#include <QStandardPaths>


namespace csv = PotatoAlert::CSV;
using namespace PotatoAlert::Time;


static constexpr std::string_view timeFormat = "%Y-%m-%d_%H-%M-%S";

QString csv::GetDir()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert/Matches");
}

std::string GetFilePath()
{
	auto dir = csv::GetDir();

	QDir d;
	d.mkpath(dir);
	d.setPath(dir);

	return d.filePath(QString::fromStdString(std::format("match_{}.csv", GetTimeStamp(timeFormat)))).toStdString();
}

void csv::SaveMatch(const std::string& csv)
{
	if (File file = File::Open(GetFilePath(), File::Flags::Write | File::Flags::Create))
	{
		if (file.Write(csv))
		{
			Logger::Debug("Wrote match as CSV.");
		}
		else
		{
			Logger::Error("Failed to save match as csv: {}", File::LastError());
		}
	}
	else
	{
		Logger::Error("Failed to open csv file for writing: {}", File::LastError());
	}

#if 0
	if (PotatoAlert::File::Write(GetFilePath(), csv))
		Logger::Debug("Wrote match as CSV.");
	else
		Logger::Error("Failed to write match as CSV.");
#endif
}

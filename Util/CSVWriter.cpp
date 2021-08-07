// Copyright 2020 <github.com/razaqq>

#include "CSVWriter.hpp"

#include "File.hpp"
#include "Log.hpp"
#include "Time.hpp"

#include <QDir>
#include <QStandardPaths>
#include <QString>

#include <format>
#include <string>


namespace csv = PotatoAlert::CSV;


static constexpr std::string_view timeFormat = "%Y-%m-%d_%H-%M-%S";

QString csv::GetDir()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert/Matches");
}

static std::string GetFilePath()
{
	auto dir = csv::GetDir();

	QDir d;
	d.mkpath(dir);
	d.setPath(dir);

	return d.filePath(QString::fromStdString(std::format("match_{}.csv", PotatoAlert::Time::GetTimeStamp(timeFormat)))).toStdString();
}

void csv::SaveMatch(const std::string& csv)
{
	if (File file = File::Open(GetFilePath(), File::Flags::Write | File::Flags::Create))
	{
		if (file.Write(csv))
		{
			LOG_TRACE("Wrote match as CSV.");
		}
		else
		{
			LOG_ERROR("Failed to save match as csv: {}", File::LastError());
		}
	}
	else
	{
		LOG_ERROR("Failed to open csv file for writing: {}", File::LastError());
	}
}
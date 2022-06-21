// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QApplication>
#include <QStandardPaths>

#include <filesystem>


namespace fs = std::filesystem;

namespace PotatoAlert::Client {

inline fs::path AppDataPath()
{
	const fs::path appData = fs::path(
		QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert").toStdString()
	);

	std::error_code ec;
	bool exists = fs::exists(appData, ec);
	if (ec)
	{
		LOG_ERROR("Failed to check for appdata path: {}", ec.message());
		QApplication::exit(1);
	}

	if (!exists)
	{
		fs::create_directories(appData, ec);
		if (ec)
		{
			LOG_ERROR("Failed to create appdata path: {}", ec.message());
			QApplication::exit(1);
		}
	}
	return appData;
}

}  // namespace PotatoAlert::Client

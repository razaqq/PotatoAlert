// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Log.hpp"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include <filesystem>
#include <string>


namespace PotatoAlert::Core {

inline std::filesystem::path AppDataPath(std::string_view appName)
{
	const QDir appData = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
		.append(QDir::separator())
		.append(appName.data()));

	if (!appData.exists())
	{
		if (!appData.mkpath("."))
		{
			LOG_ERROR("Failed to create appdata path");
			QApplication::exit(1);
		}
	}

	return appData.filesystemAbsolutePath();
}

}  // namespace PotatoAlert::Core

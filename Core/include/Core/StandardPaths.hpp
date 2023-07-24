// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Log.hpp"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include <filesystem>
#include <string>


namespace fs = std::filesystem;

namespace PotatoAlert::Core {

inline fs::path AppDataPath(std::string_view appName)
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	return appData.filesystemAbsolutePath();
#else
	const QString abs = appData.absolutePath();
	return { reinterpret_cast<const char16_t*>(abs.cbegin()), reinterpret_cast<const char16_t*>(abs.cend()) };
#endif
}

}  // namespace PotatoAlert::Core

// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Result.hpp"

#include <filesystem>

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDir>
#include <QString>
#endif


namespace PotatoAlert::Core {

Result<std::filesystem::path> GetModuleRootPath();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
inline QDir FromFilesystemPath(const std::filesystem::path& path)
{
#ifdef Q_OS_WIN
	return QString::fromStdWString(path.native());
#else
	return QString::fromStdString(path.native());
#endif
}

inline std::filesystem::path ToFilesystemAbsolutePath(const QDir& dir)
{
	const QString path = dir.absolutePath();
	return std::filesystem::path(reinterpret_cast<const char16_t*>(path.cbegin()),
								 reinterpret_cast<const char16_t*>(path.cend()));
}
#endif

static_assert(
	std::is_same_v<std::filesystem::path::value_type, char> ||
	std::is_same_v<std::filesystem::path::value_type, wchar_t>,
	"Unsupported char type std::filesystem::path::value_type"
);

}  // namespace PotatoAlert::Core

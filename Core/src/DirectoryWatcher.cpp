// Copyright 2022 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/DirectoryWatcher.hpp"

#include "Core/Log.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

#include <filesystem>
#include <ranges>
#include <string>
#include <unordered_set>


using PotatoAlert::Core::DirectoryWatcher;

DirectoryWatcher::DirectoryWatcher()
{
	connect(
		&m_watcher, &QFileSystemWatcher::directoryChanged,
		this, &DirectoryWatcher::OnDirectoryChanged
	);
}

void DirectoryWatcher::ClearDirectories()
{
	if (!m_watcher.directories().isEmpty())
		m_watcher.removePaths(m_watcher.directories());
}

void DirectoryWatcher::WatchDirectory(const std::filesystem::path& dir)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const QDir directory(dir);
#else
	const QDir directory = Core::FromFilesystemPath(dir);
#endif
	m_watcher.addPath(directory.absolutePath());

	const QFileInfoList watchedList = directory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

	for (const QFileInfo& fileInfo : watchedList)
	{
		m_lastModified[fileInfo.absoluteFilePath()] = fileInfo.lastModified();
	}
}

void DirectoryWatcher::WatchDirectory(std::string_view dir)
{
	const QString dirPath = QString(dir.data());
	m_watcher.addPath(dirPath);

	const QDir directory(dirPath);
	const QFileInfoList watchedList = directory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

	for (const QFileInfo& fileInfo : watchedList)
	{
		m_lastModified[fileInfo.absoluteFilePath()] = fileInfo.lastModified();
	}
}

void DirectoryWatcher::ForceDirectoryChanged()
{
	for (const QString& dir : m_watcher.directories())
	{
		OnDirectoryChanged(dir);
	}
}

void DirectoryWatcher::ForceFileChanged(std::string_view file)
{
	for (const QString& dir : m_watcher.directories())
	{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		emit FileChanged(QDir(dir + QDir::separator() + file.data()).filesystemAbsolutePath());
#else
		emit FileChanged(ToFilesystemAbsolutePath(dir + QDir::separator() + file.data()));
#endif
	}
}

void DirectoryWatcher::OnDirectoryChanged(const QString& path)
{
	emit DirectoryChanged(path.toStdString());

	const QDir dir(path);
	const QFileInfoList files = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

	auto beforeFiles = m_lastModified | std::views::keys | std::views::filter([&path](const QString& m) -> bool
	{
		return m.startsWith(path);
	}) | std::ranges::to<std::unordered_set<QString>>();

	for (const QFileInfo& fileInfo : files)
	{
		const QString filePath = fileInfo.absoluteFilePath();
		const QDateTime lastModified = fileInfo.lastModified();

		if (!m_lastModified.contains(filePath) || m_lastModified[filePath] < lastModified)
		{
			m_lastModified[filePath] = lastModified;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			emit FileChanged(QDir(filePath).filesystemAbsolutePath());
#else
			emit FileChanged(ToFilesystemAbsolutePath(filePath));
#endif
		}

		// remove it from the list if it still exists
		if (beforeFiles.contains(filePath))
		{
			beforeFiles.erase(filePath);
		}
	}

	// these files were removed
	for (const QString& file : beforeFiles)
	{
		m_lastModified.erase(file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		emit FileChanged(QDir(file).filesystemAbsolutePath());
#else
		emit FileChanged(ToFilesystemAbsolutePath(file));
#endif
	}
}

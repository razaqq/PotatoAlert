// Copyright 2022 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/DirectoryWatcher.hpp"

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
	const QFileInfoList watchedList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

	std::unordered_set<QString> missing(m_lastModified.size());
	for (const auto& file : m_lastModified | std::views::keys)
		missing.insert(file);

	for (const QFileInfo& fileInfo : watchedList)
	{
		QString filePath = fileInfo.absoluteFilePath();
		QDateTime lastModified = fileInfo.lastModified();

		const bool contains = m_lastModified.contains(filePath);
		if (!contains || m_lastModified[filePath] < lastModified)
		{
			m_lastModified[filePath] = lastModified;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			emit FileChanged(QDir(filePath).filesystemAbsolutePath());
#else
			emit FileChanged(ToFilesystemAbsolutePath(filePath));
#endif
		}

		if (missing.contains(filePath))
		{
			missing.erase(filePath);
		}
	}

	for (const QString& file : missing)
	{
		m_lastModified.erase(file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		emit FileChanged(QDir(file).filesystemAbsolutePath());
#else
		emit FileChanged(ToFilesystemAbsolutePath(file));
#endif
	}
}

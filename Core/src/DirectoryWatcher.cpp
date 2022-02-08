// Copyright 2022 <github.com/razaqq>

#include "Core/DirectoryWatcher.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

#include <string>
#include <unordered_set>

#include <QDebug>


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


void DirectoryWatcher::OnDirectoryChanged(const QString& path)
{
	emit DirectoryChanged(path.toStdString());

	const QDir dir(path);
	const QFileInfoList watchedList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

	std::unordered_set<QString> missing(m_lastModified.size());
	for (const auto& [file, modified] : m_lastModified)
		missing.insert(file);

	for (const QFileInfo& fileInfo : watchedList)
	{
		QString filePath = fileInfo.absoluteFilePath();
		QDateTime lastModified = fileInfo.lastModified();

		const bool contains = m_lastModified.contains(filePath);
		if (!contains || contains && m_lastModified[filePath] < lastModified)
		{
			m_lastModified[filePath] = lastModified;
			emit FileChanged(filePath.toStdString());
		}

		if (missing.contains(filePath))
		{
			missing.erase(filePath);
		}
	}

	for (const QString& file : missing)
	{
		m_lastModified.erase(file);
		emit FileChanged(file.toStdString());
	}
}

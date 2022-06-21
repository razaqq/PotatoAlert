// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QFileSystemWatcher>
#include <QDateTime>
#include <QString>

#include <string>
#include <unordered_map>


namespace PotatoAlert::Core {

class DirectoryWatcher : public QObject
{
	Q_OBJECT

public:
	DirectoryWatcher();
	void WatchDirectory(std::string_view dir);
	void ClearDirectories();
	void ForceDirectoryChanged();
	void ForceFileChanged(std::string_view file);

private:
	void OnDirectoryChanged(const QString& path);
	QFileSystemWatcher m_watcher;
	std::unordered_map<QString, QDateTime> m_lastModified;

signals:
	void DirectoryChanged(const std::string& directory);
	void FileChanged(const std::string& file);
};

}  // namespace PotatoAlert::Core

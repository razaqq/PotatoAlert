// Copyright 2020 <github.com/razaqq>

#include "Core/Log.hpp"
#include "Core/Screenshot.hpp"
#include "Core/Time.hpp"

#include <QDesktopServices>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>
#include <QWidget>

#include <format>
#include <string>


static constexpr std::string_view timeFormat = "%Y-%m-%d_%H-%M-%S";

namespace {

static QDir GetDir()
{
	QDir dir = QDir::cleanPath(
			QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
			QDir::separator() + "PotatoAlert"+ QDir::separator() + "Screenshots");
	if (!dir.exists())
	{
		LOG_TRACE("Creating screenshot directory: {}", dir.absolutePath().toStdString());
		if (!dir.mkpath("."))
		{
			LOG_ERROR("Failed to create screenshot directory");
		}
	}
	return dir;
}

static QString GetFilePath()
{
	return GetDir().filePath(QString::fromStdString(std::format("capture_{}.png", PotatoAlert::Core::Time::GetTimeStamp(timeFormat))));
}

}

bool PotatoAlert::Core::CaptureScreenshot(QWidget* window)
{
	if (!window)
		return false;

	QPixmap pix(window->size());
	window->render(&pix);
	const auto filePath = GetFilePath();
	if (pix.save(filePath, "PNG", 100))
	{
		LOG_TRACE("Saved screenshot {}", filePath.toStdString());
		QDesktopServices::openUrl(QUrl(GetDir().absolutePath()));
		return true;
	}
	LOG_ERROR("Failed to save screenshot.");
	return false;
}

// Copyright 2020 <github.com/razaqq>

#include "Screenshot.hpp"

#include "Log.hpp"
#include "Time.hpp"

#include <QDesktopServices>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>
#include <QWidget>

#include <format>
#include <string>


static constexpr std::string_view timeFormat = "%Y-%m-%d_%H-%M-%S";

namespace {

QString GetDir()
{
	auto path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert/Screenshots");
	QDir(path).mkdir(".");
	return path;
}

static QString GetFilePath()
{
	return QDir(GetDir()).filePath(QString::fromStdString(std::format("capture_{}.png", PotatoAlert::Core::Time::GetTimeStamp(timeFormat))));
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
		QDesktopServices::openUrl(QUrl(GetDir()));
		return true;
	}
	LOG_ERROR("Failed to save screenshot.");
	return false;
}

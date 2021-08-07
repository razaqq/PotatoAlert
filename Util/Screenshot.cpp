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


namespace ss = PotatoAlert::Screenshot;


static constexpr std::string_view timeFormat = "%Y-%m-%d_%H-%M-%S";

QString GetDir()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert/Screenshots");
}

static QString GetFilePath()
{
	const auto dir = GetDir();

	QDir d;
	d.mkpath(dir);
	d.setPath(dir);

	return d.filePath(QString::fromStdString(std::format("capture_{}.png", PotatoAlert::Time::GetTimeStamp(timeFormat))));
}

bool ss::Capture(QWidget* window)
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

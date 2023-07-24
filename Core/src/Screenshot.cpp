// Copyright 2020 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/Log.hpp"
#include "Core/Screenshot.hpp"
#include "Core/Time.hpp"

#include <QWidget>

#include <filesystem>
#include <format>
#include <string>


namespace fs = std::filesystem;

static constexpr std::string_view g_timeFormat = "%Y-%m-%d_%H-%M-%S";

namespace {

static std::string GetFileName()
{
	return std::format("capture_{}.png", PotatoAlert::Core::Time::GetTimeStamp(g_timeFormat));
}

}

bool PotatoAlert::Core::CaptureScreenshot(QWidget* window, const fs::path& dir)
{
	if (!window)
		return false;

	QPixmap pix(window->size());
	window->render(&pix);
	const std::string fileName = GetFileName();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	if (pix.save(QDir(dir).absoluteFilePath(fileName.c_str()), "PNG", 100))
#else
	if (pix.save(FromFilesystemPath(dir).absoluteFilePath(fileName.c_str()), "PNG", 100))
#endif
	{
		LOG_TRACE("Saved screenshot {}", fileName);
		return true;
	}
	LOG_ERROR("Failed to save screenshot.");
	return false;
}

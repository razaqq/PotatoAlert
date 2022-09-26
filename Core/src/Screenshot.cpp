// Copyright 2020 <github.com/razaqq>

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

static std::string GetFilePath(const fs::path& dir)
{
	return (dir / std::format("capture_{}.png", PotatoAlert::Core::Time::GetTimeStamp(g_timeFormat))).string();
}

}

bool PotatoAlert::Core::CaptureScreenshot(QWidget* window, const fs::path& dir)
{
	if (!window)
		return false;

	QPixmap pix(window->size());
	window->render(&pix);
	const std::string filePath = GetFilePath(dir);
	if (pix.save(filePath.c_str(), "PNG", 100))
	{
		LOG_TRACE("Saved screenshot {}", filePath);
		return true;
	}
	LOG_ERROR("Failed to save screenshot.");
	return false;
}

// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Format.hpp"

#ifdef WIN32
	#ifndef SPDLOG_WCHAR_TO_UTF8_SUPPORT
		#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
	#endif
	#ifndef SPDLOG_WCHAR_FILENAMES
		#define SPDLOG_WCHAR_FILENAMES
	#endif
#endif
#ifndef SPDLOG_FMT_EXTERNAL
	#define SPDLOG_FMT_EXTERNAL
#endif
#ifndef SPDLOG_NO_EXCEPTIONS
	#define SPDLOG_NO_EXCEPTIONS
#endif
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

#include <filesystem>
#include <string>
#include <memory>


namespace PotatoAlert::Core {

class Log
{
public:
	static void Init(const std::filesystem::path& logFile);
	static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};

}  // namespace PotatoAlert::Core

#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(::PotatoAlert::Core::Log::GetLogger(), __VA_ARGS__)
#define LOG_INFO(...)  SPDLOG_LOGGER_INFO(::PotatoAlert::Core::Log::GetLogger(), __VA_ARGS__)
#define LOG_WARN(...)  SPDLOG_LOGGER_WARN(::PotatoAlert::Core::Log::GetLogger(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(::PotatoAlert::Core::Log::GetLogger(), __VA_ARGS__)

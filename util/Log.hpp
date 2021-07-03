// Copyright 2021 <github.com/razaqq>
#pragma once

#include <memory>
#define SPDLOG_NO_EXCEPTIONS
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <QString>


namespace PotatoAlert {

class Log
{
public:
	static void Init();
	static QString GetDir();
	static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};

}

#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)
#define LOG_INFO(...)  SPDLOG_LOGGER_INFO(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)
#define LOG_WARN(...)  SPDLOG_LOGGER_WARN(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)

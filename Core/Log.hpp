// Copyright 2021 <github.com/razaqq>
#pragma once

#define SPDLOG_NO_EXCEPTIONS
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <QString>

#include <memory>


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

}  // namespace PotatoAlert

#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)
#define LOG_INFO(...)  SPDLOG_LOGGER_INFO(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)
#define LOG_WARN(...)  SPDLOG_LOGGER_WARN(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(::PotatoAlert::Log::GetLogger(), __VA_ARGS__)

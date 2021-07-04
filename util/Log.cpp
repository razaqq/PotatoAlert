// Copyright 2021 <github.com/razaqq>

#include "Log.hpp"
#include <memory>
#include <vector>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <QDir>
#include <QStandardPaths>
#include <QString>


using PotatoAlert::Log;

std::shared_ptr<spdlog::logger> Log::s_logger;

QString Log::GetDir()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert");
}

void Log::Init()
{
	auto dir = Log::GetDir();
	QDir d;
	d.mkpath(dir);
	d.setPath(dir);
	spdlog::set_error_handler([](const std::string& msg) { spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg); });

#ifndef NDEBUG
	auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdoutSink->set_pattern("%^[%T] %n: %v%$");
	stdoutSink->set_level(spdlog::level::trace);
#endif

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(d.filePath("PotatoAlert.log").toStdString());
	fileSink->set_pattern("[%d-%m-%Y %T] [%=7l] %n (%-30!@): %v");
	fileSink->set_level(spdlog::level::info);

#ifndef NDEBUG
	std::vector<spdlog::sink_ptr> sinks{ stdoutSink, fileSink };
	s_logger = std::make_shared<spdlog::logger>("PotatoAlert", sinks.begin(), sinks.end());
#else
	s_logger = std::make_shared<spdlog::logger>("PotatoAlert", fileSink);
#endif

	spdlog::register_logger(s_logger);
	s_logger->set_level(spdlog::level::trace);
	s_logger->flush_on(spdlog::level::trace);
}

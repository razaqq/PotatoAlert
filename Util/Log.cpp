// Copyright 2021 <github.com/razaqq>

#include "Log.hpp"

#pragma warning(push, 0)
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)

#include <QDir>
#include <QStandardPaths>
#include <QString>

#include <memory>
#include <vector>


using PotatoAlert::Log;

std::shared_ptr<spdlog::logger> Log::s_logger;

QString Log::GetDir()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert");
	QDir(path).mkdir(".");
	return path;
}

static void LogQtMessage(QtMsgType type, const QMessageLogContext& context, const QString& text)
{
	spdlog::level::level_enum level = spdlog::level::info;

	switch (type)
	{
		case QtDebugMsg:
			level = spdlog::level::trace;
			break;
		case QtWarningMsg:
			level = spdlog::level::warn;
			break;
		case QtCriticalMsg:
		case QtFatalMsg:
			level = spdlog::level::err;
			break;
		case QtInfoMsg:
			level = spdlog::level::info;
			break;
	}

	Log::GetLogger()->log(spdlog::source_loc{ context.file, context.line, context.function }, level, text.toStdString());
}

void Log::Init()
{
	qInstallMessageHandler(LogQtMessage);

	const std::string filePath = QDir(GetDir()).filePath("PotatoAlert.log").toStdString();
	spdlog::set_error_handler([](const std::string& msg) { spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg); });

	auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdoutSink->set_pattern("%^[%T] %n: %v%$");

#ifndef NDEBUG
	stdoutSink->set_level(spdlog::level::trace);
#else
	stdoutSink->set_level(spdlog::level::info);
#endif

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath);
	fileSink->set_pattern("[%d-%m-%Y %T] [%=7l] %n (%-30!@): %v");
	fileSink->set_level(spdlog::level::info);

	std::vector<spdlog::sink_ptr> sinks{ stdoutSink, fileSink };
	s_logger = std::make_shared<spdlog::logger>("PotatoAlert", sinks.begin(), sinks.end());

	spdlog::register_logger(s_logger);
	s_logger->set_level(spdlog::level::trace);
	s_logger->flush_on(spdlog::level::trace);
}


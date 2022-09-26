// Copyright 2021 <github.com/razaqq>

#include "Core/Log.hpp"
#include "Core/String.hpp"

#pragma warning(push, 0)
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QString>

#include <iostream>
#include <memory>
#include <vector>


using PotatoAlert::Core::String::Split;
using PotatoAlert::Core::Log;

std::shared_ptr<spdlog::logger> Log::s_logger;

namespace {

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

}

template<typename ScopedPadder>
class SourceLocationFlag : public spdlog::custom_flag_formatter
{
public:
	void format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest) override
	{
		if (msg.source.empty())
		{
			return;
		}

		const std::string baseName = spdlog::details::short_filename_formatter<
				spdlog::details::null_scoped_padder>::basename(msg.source.filename);

		size_t textSize;
		if (padinfo_.enabled())
		{
			textSize = baseName.size() + ScopedPadder::count_digits(msg.source.line) + 1;
		}
		else
		{
			textSize = 0;
		}

		ScopedPadder p(textSize, padinfo_, dest);

		spdlog::details::fmt_helper::append_string_view(baseName, dest);
		dest.push_back(':');
		spdlog::details::fmt_helper::append_int(msg.source.line, dest);
	}

	[[nodiscard]] std::unique_ptr<custom_flag_formatter> clone() const override
	{
		return spdlog::details::make_unique<SourceLocationFlag>();
	}
};

void Log::Init(std::string_view logFile)
{
	spdlog::set_error_handler([](const std::string& msg) { spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg); });

	auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdoutSink->set_pattern("%^[%T] %n: %v%$");

#ifndef NDEBUG
	stdoutSink->set_level(spdlog::level::trace);
#else
	stdoutSink->set_level(spdlog::level::info);
#endif

	auto formatter = std::make_unique<spdlog::pattern_formatter>();
	formatter->add_flag<SourceLocationFlag<spdlog::details::scoped_padder>>('S');
	formatter->set_pattern("[%d-%m-%Y %T] [%=7l] %n [thread %-5t] (%-30!S): %v");

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile.data());
	fileSink->set_formatter(std::move(formatter));
	fileSink->set_level(spdlog::level::info);

	std::vector<spdlog::sink_ptr> sinks{ stdoutSink, fileSink };
	s_logger = std::make_shared<spdlog::logger>("PotatoAlert", sinks.begin(), sinks.end());

	spdlog::register_logger(s_logger);
	s_logger->set_level(spdlog::level::trace);
	s_logger->flush_on(spdlog::level::trace);

	qInstallMessageHandler(LogQtMessage);
}

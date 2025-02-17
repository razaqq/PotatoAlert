// Copyright 2021 <github.com/razaqq>

#include "Core/Log.hpp"

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>


using PotatoAlert::Core::Log;

std::shared_ptr<spdlog::logger> Log::s_logger;

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

void Log::Init(const std::filesystem::path& logFile)
{
	spdlog::set_error_handler([](const std::string& msg)
	{
		spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg);
	});

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

	const auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile.native());
	fileSink->set_formatter(std::move(formatter));
	fileSink->set_level(spdlog::level::info);

	std::vector<spdlog::sink_ptr> sinks{ stdoutSink, fileSink };
	s_logger = std::make_shared<spdlog::logger>("PotatoAlert", sinks.begin(), sinks.end());

	spdlog::register_logger(s_logger);
	s_logger->set_level(spdlog::level::trace);
	s_logger->flush_on(spdlog::level::trace);
}

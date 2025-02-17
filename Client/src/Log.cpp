// Copyright 2025 <github.com/razaqq>

#include "Client/Log.hpp"

#include "Core/Log.hpp"

#include <QMessageLogContext>
#include <QString>
#include <QtGlobal>


void PotatoAlert::Client::LogQtMessage(QtMsgType type, const QMessageLogContext& context, const QString& text)
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

	Core::Log::GetLogger()->log(spdlog::source_loc{ context.file, context.line, context.function }, level, text.toStdString());
}

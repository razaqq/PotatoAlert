// Copyright 2025 <github.com/razaqq>
#pragma once

#include <QMessageLogContext>
#include <QString>
#include <QtGlobal>


namespace PotatoAlert::Client {

void LogQtMessage(QtMsgType type, const QMessageLogContext& context, const QString& text);

}  // namespace PotatoAlert::Client

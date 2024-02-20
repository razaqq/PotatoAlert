// Copyright 2020 <github.com/razaqq>

#include "Gui/FramelessDialog.hpp"

#include <QDialog>
#include <QWidget>
#include <QWindow>


using PotatoAlert::Gui::FramelessDialog;

void FramelessDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool FramelessDialog::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
#else
bool FramelessDialog::nativeEvent(const QByteArray& eventType, void* message, long* result)
#endif
{
	return QDialog::nativeEvent(eventType, message, result);
}

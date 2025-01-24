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

bool FramelessDialog::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
	return QDialog::nativeEvent(eventType, message, result);
}

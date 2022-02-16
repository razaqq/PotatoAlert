// Copyright 2020 <github.com/razaqq>

#include "Gui/FramelessDialog.hpp"

#include <QDialog>
#include <QWidget>
#include <QWindow>


using PotatoAlert::Gui::FramelessDialog;

FramelessDialog::FramelessDialog(QWidget* parent) : QDialog(parent)
{
	this->setParent(parent);

	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
	this->setWindowModality(Qt::WindowModal);
}

void FramelessDialog::showEvent(QShowEvent* event)
{
}

bool FramelessDialog::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	return true;
}

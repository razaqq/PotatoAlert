// Copyright 2020 <github.com/razaqq>

#include "Gui/SettingsWidget/HorizontalLine.hpp"

#include <QFrame>
#include <QWidget>


using PotatoAlert::Gui::HorizontalLine;

HorizontalLine::HorizontalLine(QWidget* parent) : QFrame(parent)
{
	setObjectName("hLine");
	setFrameShape(QFrame::HLine);
	setFrameShadow(QFrame::Plain);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

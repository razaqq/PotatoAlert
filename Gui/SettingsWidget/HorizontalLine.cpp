// Copyright 2020 <github.com/razaqq>

#include "HorizontalLine.hpp"

#include <QFrame>
#include <QWidget>


using PotatoAlert::Gui::HorizontalLine;

HorizontalLine::HorizontalLine(QWidget* parent) : QFrame(parent)
{
	this->setObjectName("hLine");
	this->setFrameShape(QFrame::HLine);
	this->setFrameShadow(QFrame::Plain);
}

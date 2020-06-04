// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QFrame>
#include "HorizontalLine.h"


using PotatoAlert::HorizontalLine;

HorizontalLine::HorizontalLine(QWidget* parent) : QFrame(parent)
{
	this->setObjectName("hLine");
	this->setFrameShape(QFrame::HLine);
	this->setFrameShadow(QFrame::Plain);
}

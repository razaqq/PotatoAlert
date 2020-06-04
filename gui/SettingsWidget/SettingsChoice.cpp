// Copyright 2020 <github.com/razaqq>

#include "SettingsChoice.h"
#include <QWidget>
#include <QString>
#include <QFont>
#include <QPushButton>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QSizePolicy>


const int WIDGETHEIGHT = 20;

using PotatoAlert::SettingsChoice;

SettingsChoice::SettingsChoice(QWidget* parent, std::vector<QString>& buttons)
{
	this->setObjectName("settingsChoice");

	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	this->setCursor(Qt::PointingHandCursor);
	this->setFixedHeight(WIDGETHEIGHT);
	// this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	this->btnGroup = new QButtonGroup(this);
	this->btnGroup->setExclusive(true);

	QFont btnFont("Helvetica Neue", 10, QFont::DemiBold);
	btnFont.setStyleStrategy(QFont::PreferAntialias);

	for (int i = 0; i < buttons.size(); i++)
	{
		QPushButton* button = new QPushButton(buttons[i], this);
		button->setObjectName("settingsChoiceButton");
		button->setMinimumWidth(5);
		button->setFont(btnFont);
		button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		int width = button->fontMetrics().boundingRect(buttons[i]).width() + 10;
		button->setFixedWidth(width);
		button->setFixedHeight(WIDGETHEIGHT);
		button->setFlat(true);
		button->setCheckable(true);

		this->btnGroup->addButton(button, i);
		layout->addWidget(button);
	}
	this->setLayout(layout);
}

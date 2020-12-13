// Copyright 2020 <github.com/razaqq>

#include "SettingsChoice.hpp"
#include <QWidget>
#include <QString>
#include <QFont>
#include <QPushButton>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QSizePolicy>


const int WIDGET_HEIGHT = 20;

using PotatoAlert::SettingsChoice;

SettingsChoice::SettingsChoice(QWidget* parent, const std::vector<QString>& buttons) : QWidget(parent)
{
	this->setObjectName("settingsChoice");

    auto layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	this->setCursor(Qt::PointingHandCursor);
	this->setFixedHeight(WIDGET_HEIGHT);

	this->btnGroup = new QButtonGroup(this);
	this->btnGroup->setExclusive(true);

	QFont btnFont("Helvetica Neue", 10, QFont::DemiBold);
	btnFont.setStyleStrategy(QFont::PreferAntialias);

	for (int i = 0; i < buttons.size(); i++)
	{
        auto button = new QPushButton(buttons[i], this);
		button->setObjectName("settingsChoiceButton");
		button->setMinimumWidth(5);
		button->setFont(btnFont);
		button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		int width = button->fontMetrics().boundingRect(buttons[i]).width() + 10;
		button->setFixedWidth(width);
		button->setFixedHeight(WIDGET_HEIGHT);
		button->setFlat(true);
		button->setCheckable(true);

		this->btnGroup->addButton(button, i);
		layout->addWidget(button);
	}
	this->setLayout(layout);
}

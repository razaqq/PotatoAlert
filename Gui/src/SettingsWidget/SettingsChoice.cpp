// Copyright 2020 <github.com/razaqq>

#include "Gui/SettingsWidget/SettingsChoice.hpp"

#include <QButtonGroup>
#include <QFont>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QString>
#include <QWidget>


const int WIDGET_HEIGHT = 20;

using PotatoAlert::Gui::SettingsChoice;

SettingsChoice::SettingsChoice(QWidget* parent, const std::vector<QString>& buttons) : QWidget(parent)
{
	this->setObjectName("settingsChoice");

	auto layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	this->setCursor(Qt::PointingHandCursor);
	this->setFixedHeight(WIDGET_HEIGHT);

	this->m_btnGroup = new QButtonGroup(this);
	this->m_btnGroup->setExclusive(true);

	QFont btnFont("Helvetica Neue", 10, QFont::DemiBold);
	btnFont.setStyleStrategy(QFont::PreferAntialias);

	for (size_t i = 0; i < buttons.size(); i++)
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

		this->m_btnGroup->addButton(button, static_cast<int>(i));
		layout->addWidget(button);
	}
	this->setLayout(layout);
}

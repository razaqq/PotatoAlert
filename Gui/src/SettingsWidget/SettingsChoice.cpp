// Copyright 2020 <github.com/razaqq>

#include "Gui/SettingsWidget/SettingsChoice.hpp"

#include <QButtonGroup>
#include <QFont>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSizePolicy>
#include <QString>
#include <QStyle>
#include <QWidget>


constexpr int WIDGET_HEIGHT = 20;

using PotatoAlert::Gui::SettingsChoice;

SettingsChoice::SettingsChoice(QWidget* parent, std::initializer_list<const QString> buttons) : QWidget(parent)
{
	Init(buttons.begin(), buttons.end());
}

SettingsChoice::SettingsChoice(QWidget* parent, std::span<const QString> buttons) : QWidget(parent)
{
	Init(buttons.begin(), buttons.end());
}

template<typename Iterator>
void SettingsChoice::Init(Iterator begin, Iterator end)
{
	setObjectName("settingsChoice");

	auto layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(1);

	setCursor(Qt::PointingHandCursor);
	setFixedHeight(WIDGET_HEIGHT);

	m_btnGroup = new QButtonGroup(this);
	m_btnGroup->setExclusive(true);

	// QFont btnFont("Helvetica Neue", 10, QFont::DemiBold);
	QFont btnFont("Noto Sans", 10, QFont::DemiBold);
	btnFont.setStyleStrategy(QFont::PreferAntialias);

	size_t i = 0;
	for (auto it = begin; it != end; it++)
	{
		auto button = new SettingsChoiceButton(*it, this);

		static constexpr const char* PosProp = "GroupPosition";
		if (it == begin)
			button->setProperty(PosProp, "First");
		else if (it == end-1)
			button->setProperty(PosProp, "Last");
		else if (begin+1 == end)
			button->setProperty(PosProp, "Single");
		else
			button->setProperty(PosProp, "Middle");

		button->setObjectName("settingsChoiceButton");
		button->setMinimumWidth(5);
		button->setFont(btnFont);
		button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		int width = button->fontMetrics().boundingRect(*it).width() + 10;
		button->setFixedWidth(width);
		button->setFixedHeight(WIDGET_HEIGHT);
		button->setFlat(true);
		button->setCheckable(true);

		m_btnGroup->addButton(button, static_cast<int>(i));
		layout->addWidget(button);
		i++;
	}

	setLayout(layout);
}

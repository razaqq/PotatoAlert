// Copyright 2020 <github.com/razaqq>

#include "MenuEntryButton.hpp"

#include <QIcon>
#include <QLayout>
#include <QPushButton>
#include <QSize>
#include <QVBoxLayout>
#include <QWidget>


using PotatoAlert::Gui::MenuEntryButton;

MenuEntryButton::MenuEntryButton(QWidget* parent, const QIcon& icon, bool checkable) : QWidget(parent)
{
	this->setObjectName("menuEntry");

	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(5);

	this->setFixedWidth(parent->width());

	this->m_button->setIcon(icon);
	int width = this->width() - 2 * layout->spacing();
	this->m_button->setIconSize(QSize(width, width));
	this->m_button->setCursor(Qt::PointingHandCursor);
	this->m_button->setCheckable(checkable);
	this->m_button->setFlat(true);
	layout->addWidget(this->m_button);

	this->setLayout(layout);
}

void MenuEntryButton::SetChecked(bool checked) const
{
	this->m_button->setChecked(checked);
}


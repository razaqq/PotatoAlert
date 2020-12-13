// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QIcon>
#include <QPushButton>
#include <QLayout>
#include <QSize>
#include <QVBoxLayout>
#include <QDebug>
#include "MenuEntry.hpp"


using PotatoAlert::MenuEntry;

MenuEntry::MenuEntry(QWidget* parent, const QIcon& icon) : QWidget(parent)
{
    this->setObjectName("menuEntry");

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    this->setFixedWidth(parent->width());

    this->button->setIcon(icon);
    int width = this->width() - 2 * layout->spacing();
    this->button->setIconSize(QSize(width, width));
    this->button->setCursor(Qt::PointingHandCursor);
    this->button->setCheckable(true);
    this->button->setFlat(true);
    layout->addWidget(this->button);

    this->setLayout(layout);
}

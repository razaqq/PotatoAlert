// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QIcon>
#include <QPushButton>


namespace PotatoAlert {

class MenuEntry : public QWidget
{
public:
	MenuEntry(QWidget* parent, const QIcon& icon);
	QPushButton* button = new QPushButton;
};

}  // namespace PotatoAlert

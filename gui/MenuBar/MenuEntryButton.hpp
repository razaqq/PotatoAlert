// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QIcon>
#include <QPushButton>


namespace PotatoAlert {

class MenuEntryButton : public QWidget
{
public:
	MenuEntryButton(QWidget* parent, const QIcon& icon, bool checkable = true);
	QPushButton* button = new QPushButton;
};

}  // namespace PotatoAlert

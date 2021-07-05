// Copyright 2020 <github.com/razaqq>
#pragma once

#include "MenuEntryButton.hpp"

#include <QButtonGroup>
#include <QDockWidget>
#include <QWidget>

#include <array>


namespace PotatoAlert {

enum class MenuEntry
{
	Table,
	Settings,
	Discord,
	CSV,
	Log,
	Github,
	About,
};

class VerticalMenuBar : public QDockWidget
{
	Q_OBJECT
public:
	explicit VerticalMenuBar(QWidget* parent = nullptr);
	void SetChecked(MenuEntry entry);
private:
	std::array<MenuEntryButton*, 7> m_menuEntries;
	QButtonGroup* m_btnGroup = new QButtonGroup();
	void Init();
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void EntryClicked(MenuEntry entry);
#pragma clang diagnostic pop
};

}  // namespace PotatoAlert

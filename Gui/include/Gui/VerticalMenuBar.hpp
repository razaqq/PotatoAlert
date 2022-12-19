// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Gui/IconButton.hpp"

#include <QButtonGroup>
#include <QDockWidget>
#include <QWidget>

#include <array>


namespace PotatoAlert::Gui {

enum class MenuEntry
{
	Table,
	Settings,
	Discord,
	MatchHistory,
	
	Screenshot,
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
	void SetChecked(MenuEntry entry) const;
private:
	std::array<IconButton*, 9> m_menuEntries;
	QButtonGroup* m_btnGroup = new QButtonGroup();
	void Init();
signals:
	void EntryClicked(MenuEntry entry);
};

}  // namespace PotatoAlert::Gui

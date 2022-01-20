// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QEvent>
#include <QTableWidget>
#include <QWidget>


namespace PotatoAlert::Gui {

class StatsTable : public QTableWidget
{
public:
	explicit StatsTable(QWidget* parent = nullptr);
private:
	void Init();
	void InitHeaders();
	void changeEvent(QEvent* event) override;
};

}  // namespace PotatoAlert::Gui

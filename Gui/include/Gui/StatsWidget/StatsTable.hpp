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
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
	void InitHeaders();
};

}  // namespace PotatoAlert::Gui

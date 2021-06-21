// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QTableWidget>
#include <QEvent>
#include <QWidget>


namespace PotatoAlert {

class StatsTable : public QTableWidget
{
public:
	explicit StatsTable(QWidget* parent);
private:
	void Init();
	void InitHeaders();
	void changeEvent(QEvent* event) override;
};

}  // namespace PotatoAlert

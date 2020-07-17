// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QTableWidget>
#include <QWidget>
#include <QMouseEvent>
#include <QString>
#include <QEvent>
#include <vector>


namespace PotatoAlert {

class StatsTable : public QTableWidget
{
public:
	explicit StatsTable(QWidget* parent);
	void setWowsNumbers(const std::vector<QString>& wowsNumbers);
private:
	void init();
	void initHeaders();
	void clickEvent(int row, int column);
	void changeEvent(QEvent* event);

	std::vector<QString> wowsNumbers;
};

}  // namespace PotatoAlert

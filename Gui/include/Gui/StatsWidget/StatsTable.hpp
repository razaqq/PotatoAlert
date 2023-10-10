// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include <QEvent>
#include <QTableWidget>
#include <QWidget>


namespace PotatoAlert::Gui {

class StatsTable : public QTableWidget
{
public:
	explicit StatsTable(bool showKarma, QWidget* parent = nullptr);
	bool eventFilter(QObject* watched, QEvent* event) override;
	void SetShowKarma(bool showKarma);

private:
	void Init();
	void InitHeaders();

private:
	bool m_showKarma;
};

}  // namespace PotatoAlert::Gui

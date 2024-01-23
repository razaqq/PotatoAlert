// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QEvent>
#include <QTableView>


namespace PotatoAlert::Gui {

class MatchHistoryView : public QTableView
{
public:
	MatchHistoryView()
	{
		qApp->installEventFilter(this);
	}

	void Init();
	bool eventFilter(QObject* watched, QEvent* event) override;
};

}  // namespace PotatoAlert::Gui

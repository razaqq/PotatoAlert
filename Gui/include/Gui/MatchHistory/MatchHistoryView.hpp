// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/StatsParser.hpp"

#include "Gui/MatchHistory/MatchHistoryModel.hpp"
#include "Gui/MatchHistory/MatchHistoryView.hpp"

#include <QHeaderView>
#include <QTableView>


namespace PotatoAlert::Gui {

class MatchHistoryView : public QTableView
{
public:
	MatchHistoryView()
	{
		qApp->installEventFilter(this);
	}

	void Init()
	{
		setObjectName("matchHistoryTable");
		viewport()->setAttribute(Qt::WA_Hover);
		setMinimumWidth(900);
		setCursor(Qt::PointingHandCursor);

		setSelectionBehavior(SelectRows);
		setSelectionMode(ExtendedSelection);
		setDragDropMode(NoDragDrop);

		horizontalHeader()->setVisible(true);
		horizontalHeader()->setHighlightSections(false);
		horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		horizontalHeader()->setSectionResizeMode(MatchHistoryModel::ButtonColumn(), QHeaderView::Fixed);
		horizontalHeader()->setMinimumSectionSize(20);
		horizontalHeader()->resizeSection(MatchHistoryModel::ButtonColumn(), 20);

		verticalHeader()->setVisible(false);
		verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		verticalHeader()->setDefaultSectionSize(20);
	}
};

}  // namespace PotatoAlert::Gui

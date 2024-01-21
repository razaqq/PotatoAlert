// Copyright 2023 <github.com/razaqq>

#include "Gui/Events.hpp"
#include "Gui/Fonts.hpp"
#include "Gui/MatchHistory/MatchHistoryModel.hpp"
#include "Gui/MatchHistory/MatchHistoryView.hpp"

#include <QHeaderView>
#include <QTableView>


using PotatoAlert::Gui::MatchHistoryView;

void MatchHistoryView::Init()
{
	setObjectName("matchHistoryTable");
	viewport()->setAttribute(Qt::WA_Hover);
	setCursor(Qt::PointingHandCursor);

	setSelectionBehavior(SelectRows);
	setSelectionMode(ExtendedSelection);
	setDragDropMode(NoDragDrop);

	horizontalHeader()->setVisible(true);
	horizontalHeader()->setHighlightSections(false);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(MatchHistoryModel::ButtonColumn(), QHeaderView::Fixed);
	horizontalHeader()->setMinimumSectionSize(20);
	horizontalHeader()->resizeSection(MatchHistoryModel::ButtonColumn(), 20);

	verticalHeader()->setVisible(false);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setDefaultSectionSize(20);
}

bool MatchHistoryView::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == FontScalingChangeEvent::RegisteredType())
	{
		const float scaling = dynamic_cast<FontScalingChangeEvent*>(event)->GetScaling();
		QFont f = font();
		f.setPointSizeF(9.0f * scaling);
		setFont(f);

		setMinimumWidth((int)std::roundf(800.0f * scaling));
	}
	return QTableView::eventFilter(watched, event);
}

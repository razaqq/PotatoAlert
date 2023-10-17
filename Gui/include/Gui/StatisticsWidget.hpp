// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"

#include "Gui/MatchHistory/MatchHistory.hpp"
#include "Gui/MatchHistory/MatchHistoryModel.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QWidget>


namespace PotatoAlert::Gui {

class StatWidget : public QWidget
{
	Q_OBJECT

public:
	StatWidget(std::span<const Client::Match> matches, QWidget* parent = nullptr);
};

class StatisticsWidget : public QWidget
{
	Q_OBJECT

public:
	StatisticsWidget(const MatchHistory* matchHistory, QWidget* parent = nullptr);

	void RefreshAll();
	void RefreshLatest();

private:
	const MatchHistory* m_matchHistory;
};

}  // namespace PotatoAlert::Gui

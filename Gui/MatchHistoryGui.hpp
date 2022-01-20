// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Client/MatchHistory.hpp"
#include "Client/StatsParser.hpp"

#include <QEvent>
#include <QTableWidget>
#include <QWidget>


using PotatoAlert::Client::MatchHistory;

namespace PotatoAlert::Gui {

class MatchHistoryGui : public QWidget
{
	Q_OBJECT
public:
	explicit MatchHistoryGui(QWidget* parent = nullptr);
	void UpdateAll();
	void UpdateLatest();

private:
	void Init();
	void InitHeaders() const;
	void changeEvent(QEvent* event) override;
	void paintEvent(QPaintEvent* _) override;
	QTableWidget* m_table = new QTableWidget();
	void AddEntry(const MatchHistory::MatchHistoryEntry& entry) const;
	int m_latestId = 0;

signals:
	void ReplaySelected(const Client::StatsParser::Match& match);
};

}  // namespace PotatoAlert::Gui

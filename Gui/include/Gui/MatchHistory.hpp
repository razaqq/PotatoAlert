// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Client/MatchHistory.hpp"
#include "Client/StatsParser.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QEvent>
#include <QTableWidget>
#include <QWidget>

#include <map>


namespace PotatoAlert::Gui {

class MatchHistory : public QWidget
{
	Q_OBJECT
public:
	explicit MatchHistory(QWidget* parent = nullptr);
	void UpdateAll();
	void UpdateLatest();
	void SetSummary(uint32_t id, const ReplaySummary& summary) const;

private:
	void Init();
	void InitHeaders() const;
	void changeEvent(QEvent* event) override;
	void paintEvent(QPaintEvent* _) override;
	QTableWidget* m_table = new QTableWidget();
	void AddEntry(const Client::MatchHistory::Entry& entry);
	std::map<uint32_t, std::array<QTableWidgetItem*, 7>> m_entries;

signals:
	void ReplaySelected(const Client::StatsParser::Match& match);
};

}  // namespace PotatoAlert::Gui

// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Client/MatchHistory.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/IconButton.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QEvent>
#include <QPushButton>
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
	void SetSummary(uint32_t id, const ReplayParser::ReplaySummary& summary) const;

private:
	struct GuiEntry
	{
		std::array<QTableWidgetItem*, 7> Fields;
		IconButton* Button;
	};

	void Init();
	void InitHeaders() const;
	void changeEvent(QEvent* event) override;
	void paintEvent(QPaintEvent* _) override;
	QTableWidget* m_table = new QTableWidget();
	void AddEntry(const Client::MatchHistory::Entry& entry);
	std::map<uint32_t, GuiEntry> m_entries;
	QPushButton* m_deleteButton = new QPushButton();
	int m_btnColumn;
	int m_jsonColumn;
	// QPushButton* m_analyzeButton = new QPushButton();

signals:
	void ReplaySelected(const Client::StatsParser::MatchType& match);
	void ReplaySummarySelected(const Client::MatchHistory::Entry& entry);
};

}  // namespace PotatoAlert::Gui

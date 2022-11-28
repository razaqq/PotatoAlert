// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"
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
private:
	const Client::ServiceProvider& m_services;

	struct GuiEntry
	{
		std::array<QTableWidgetItem*, 7> Fields;
		IconButton* Button;
	};

	QTableWidget* m_table = new QTableWidget();
	std::map<uint32_t, GuiEntry> m_entries;
	QPushButton* m_deleteButton = new QPushButton();
	int m_btnColumn;
	int m_jsonColumn;

public:
	explicit MatchHistory(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);
	void UpdateAll();
	void UpdateLatest();
	void SetSummary(uint32_t id, const ReplayParser::ReplaySummary& summary) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
	void InitHeaders() const;
	void paintEvent(QPaintEvent* _) override;
	void AddMatch(const Client::Match& match);

signals:
	void ReplaySelected(const Client::StatsParser::MatchType& match);
	void ReplaySummarySelected(const Client::Match& match);
};

}  // namespace PotatoAlert::Gui

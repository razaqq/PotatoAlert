// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Serializer.hpp"
#include "StatsParser.hpp"

#include <QEvent>
#include <QTableWidget>
#include <QWidget>


namespace PotatoAlert {

class MatchHistory : public QWidget
{
	Q_OBJECT
public:
	explicit MatchHistory(QWidget* parent = nullptr);
	void UpdateAll();
	void UpdateLatest();

private:
	void Init();
	void InitHeaders() const;
	void changeEvent(QEvent* event) override;
	void paintEvent(QPaintEvent* _) override;
	QTableWidget* m_table = new QTableWidget();
	void AddEntry(const Serializer::MatchHistoryEntry& entry) const;
	int m_latestId = 0;

signals:
	void ReplaySelected(const StatsParser::Match& match);
};

}  // namespace PotatoAlert

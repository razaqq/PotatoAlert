// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/StatsParser.hpp"

#include <QLabel>
#include <QWidget>


namespace PotatoAlert::Gui {

using Client::StatsParser::Team;

class StatsTeamFooter : public QWidget
{
private:
	QLabel* m_wrLabel = new QLabel();
	QLabel* m_dmgLabel = new QLabel();
	QLabel* m_regionLabel = new QLabel();

	QLabel* m_wr = new QLabel("0.0%");
	QLabel* m_dmg = new QLabel("0");
	QLabel* m_tag = new QLabel();
	QLabel* m_name = new QLabel();
	QLabel* m_region = new QLabel();

public:
	explicit StatsTeamFooter(QWidget* parent = nullptr);
	void Update(const Team& team) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
};

}  // namespace PotatoAlert::Gui

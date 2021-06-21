// Copyright 2020 <github.com/razaqq>
#pragma once

#include "StatsParser.hpp"
#include <QLabel>
#include <QString>
#include <QWidget>


namespace PotatoAlert {

using PotatoAlert::StatsParser::Match;

class StatsTeamFooter : public QWidget
{
public:
	explicit StatsTeamFooter(QWidget* parent);
	void Update(const Match& match);
private:
	void Init();
	void changeEvent(QEvent* event) override;

	QLabel* team1WrLabel = new QLabel();
	QLabel* team1DmgLabel = new QLabel();
	QLabel* team2WrLabel = new QLabel();
	QLabel* team2DmgLabel = new QLabel();
	QLabel* team1RegionLabel = new QLabel();
	QLabel* team2RegionLabel = new QLabel();

	QLabel* team1Wr = new QLabel("0.0%");
	QLabel* team1Dmg = new QLabel("0");
	QLabel* team1Tag = new QLabel();
	QLabel* team1Name = new QLabel();
	QLabel* team1Region = new QLabel();

	QLabel* team2Wr = new QLabel("0.0%");
	QLabel* team2Dmg = new QLabel("0");
	QLabel* team2Tag = new QLabel();
	QLabel* team2Name = new QLabel();
	QLabel* team2Region = new QLabel();
};

}  // namespace PotatoAlert

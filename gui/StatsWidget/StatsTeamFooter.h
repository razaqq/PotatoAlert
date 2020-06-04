// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>


namespace PotatoAlert {

class StatsTeamFooter : public QWidget
{
public:
	explicit StatsTeamFooter(QWidget* parent);
	void setAverages(const std::vector<QString>& avg);
	void setClans(const std::vector<QString>& clans);
private:
	void init();

	QLabel* team1Wr = new QLabel("");
	QLabel* team1Dmg = new QLabel("");
	QLabel* team1Tag = new QLabel("[RUUU]");
	QLabel* team1Name = new QLabel("dfgdfgdfg asdasd");
	QLabel* team1Region = new QLabel("ru");

	QLabel* team2Wr = new QLabel("");
	QLabel* team2Dmg = new QLabel("");
	QLabel* team2Tag = new QLabel("[HYDRO]");
	QLabel* team2Name = new QLabel("poor mans radar");
	QLabel* team2Region = new QLabel("eu");

	QLabel* region1Label = new QLabel("Region: ");
	QLabel* region2Label = new QLabel("Region: ");
};

}  // namespace PotatoAlert

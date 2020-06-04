// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QString>
#include <iostream>
#include <vector>
#include "StatsTeamFooter.h"


using PotatoAlert::StatsTeamFooter;

StatsTeamFooter::StatsTeamFooter(QWidget* parent) : QWidget(parent)
{
	init();
	// this->setStyleSheet("border: 1px solid red;");
}

void StatsTeamFooter::init()
{
	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(10, 0, 10, 0);
	layout->setSpacing(10);

	const QFont labelFont = QFont("Segoe UI", 10, QFont::Bold);

	// left side
	QWidget* leftWidget = new QWidget;
	QHBoxLayout* leftLayout = new QHBoxLayout;
	leftLayout->setContentsMargins(10, 0, 10, 0);
	leftLayout->setSpacing(20);

	// right side
	QWidget* rightWidget = new QWidget;
	QHBoxLayout* rightLayout = new QHBoxLayout;
	rightLayout->setContentsMargins(10, 0, 10, 0);
	rightLayout->setSpacing(20);

	// set font on all labels
	std::vector<std::vector<QLabel*>> labels{
		std::vector<QLabel*>{ new QLabel("WR: "), this->team1Wr, new QLabel("DMG: "), this->team1Dmg, this->team1Tag, this->team1Name, this->region1Label, this->team1Region },
		std::vector<QLabel*>{ new QLabel("WR: "), this->team2Wr, new QLabel("DMG: "), this->team2Dmg, this->team2Tag, this->team2Name, this->region2Label, this->team2Region }
	};
	for (auto side : labels)
		for (auto label : side)
			label->setFont(labelFont);

	// add labels
	for (int side = 0; side < 2; side++)
	{
		for (size_t element = 0; element < 4; element++)
		{
			QWidget* w = new QWidget;
			QHBoxLayout* l = new QHBoxLayout;
			l->setContentsMargins(0, 0, 0, 0);
			l->setSpacing(0);

			l->addWidget(labels[side][2*element], 0, Qt::AlignRight);
			l->addWidget(labels[side][2*element+1], 0, Qt::AlignRight);

			w->setLayout(l);

			if (side == 0) {
				leftLayout->addWidget(w);
				if (element == 1 || element == 2)
					leftLayout->addStretch();
			}
			else
			{
				rightLayout->addWidget(w);
				if (element == 1 || element == 2)
					rightLayout->addStretch();
			}
		}
	}

	leftWidget->setLayout(leftLayout);
	rightWidget->setLayout(rightLayout);
	layout->addWidget(leftWidget);
	layout->addWidget(rightWidget);
	this->setLayout(layout);
}

void StatsTeamFooter::setAverages(const std::vector<QString>& avg)
{
	// t1Wr, t1WrStyle, t1Dmg, t1DmgStyle, t2Wr, t2WrStyle, t2Dmg, t2DmgStyle
	this->team1Wr->setText(avg[0]);
	this->team1Wr->setStyleSheet(avg[1]);

	this->team1Dmg->setText(avg[2]);
	this->team1Dmg->setStyleSheet(avg[3]);

	this->team2Wr->setText(avg[4]);
	this->team2Wr->setStyleSheet(avg[5]);

	this->team2Dmg->setText(avg[6]);
	this->team2Dmg->setStyleSheet(avg[7]);
}

void StatsTeamFooter::setClans(const std::vector<QString>& clans)
{
	// t1Tag, t1TagStyle, t1Name, t1Region, t2Tag, t2TagStyle, t2Name, t2Region
	if (clans.size() == 8)
	{
		this->team1Tag->setText(clans[0]);
		this->team1Tag->setStyleSheet(clans[1]);
		this->team1Name->setText(clans[2]);
		this->team1Region->setText(clans[3]);

		this->team1Tag->setVisible(true);
		this->team1Name->setVisible(true);
		this->team1Region->setVisible(true);
		this->region1Label->setVisible(true);

		this->team2Tag->setText(clans[4]);
		this->team2Tag->setStyleSheet(clans[5]);
		this->team2Name->setText(clans[6]);
		this->team2Region->setText(clans[7]);

		this->team2Tag->setVisible(true);
		this->team2Name->setVisible(true);
		this->team2Region->setVisible(true);
		this->region2Label->setVisible(true);
	}
	else
	{

		this->team1Tag->setVisible(false);
		this->team1Name->setVisible(false);
		this->team1Region->setVisible(false);
		this->region1Label->setVisible(false);

		this->team2Tag->setVisible(false);
		this->team2Name->setVisible(false);
		this->team2Region->setVisible(false);
		this->region2Label->setVisible(false);
	}
}

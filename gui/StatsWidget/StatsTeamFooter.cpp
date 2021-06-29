// Copyright 2020 <github.com/razaqq>

#include "StatsTeamFooter.hpp"
#include "StatsParser.hpp"
#include "StringTable.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QWidget>
#include <iostream>
#include <vector>


using PotatoAlert::StatsTeamFooter;

StatsTeamFooter::StatsTeamFooter(QWidget* parent) : QWidget(parent)
{
	this->Init();
}

void StatsTeamFooter::Init()
{
	auto layout = new QHBoxLayout;
	layout->setContentsMargins(10, 0, 10, 0);
	layout->setSpacing(10);

	const QFont labelFont = QFont("Segoe UI", 10, QFont::Bold);

	// left side
	auto leftWidget = new QWidget;
	auto leftLayout = new QHBoxLayout;
	leftLayout->setContentsMargins(10, 0, 10, 0);
	leftLayout->setSpacing(20);

	// right side
	auto rightWidget = new QWidget;
	auto rightLayout = new QHBoxLayout;
	rightLayout->setContentsMargins(10, 0, 10, 0);
	rightLayout->setSpacing(20);

	// set font on all labels
	std::vector<std::vector<QLabel*>> labels{
		std::vector<QLabel*>{ this->team1WrLabel, this->team1Wr, this->team1DmgLabel, this->team1Dmg, this->team1Tag, this->team1Name, this->team1RegionLabel, this->team1Region },
		std::vector<QLabel*>{ this->team2WrLabel, this->team2Wr, this->team2DmgLabel, this->team2Dmg, this->team2Tag, this->team2Name, this->team2RegionLabel, this->team2Region }
	};
	for (auto& side : labels)
		for (auto& label : side)
			label->setFont(labelFont);

	// add labels
	for (size_t side = 0; side < 2; side++)
	{
		for (size_t element = 0; element < 4; element++)
		{
			auto w = new QWidget;
			auto l = new QHBoxLayout;
			l->setContentsMargins(0, 0, 0, 0);
			l->setSpacing(0);

			l->addWidget(labels[side][2*element], 0, Qt::AlignRight);
			l->addWidget(labels[side][2*element+1], 0, Qt::AlignRight);

			w->setLayout(l);

			if (side == 0)
			{
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

	this->team1RegionLabel->setVisible(false);
	this->team2RegionLabel->setVisible(false);

	leftWidget->setLayout(leftLayout);
	rightWidget->setLayout(rightLayout);
	layout->addWidget(leftWidget);
	layout->addWidget(rightWidget);
	this->setLayout(layout);
}

void StatsTeamFooter::Update(const Match& match)
{
	// set average stats per team
	match.team1.winrate.UpdateLabel(this->team1Wr);
	match.team1.avgDmg.UpdateLabel(this->team1Dmg);
	match.team2.winrate.UpdateLabel(this->team2Wr);
	match.team2.avgDmg.UpdateLabel(this->team2Dmg);

	// set clan battle stuff
	bool show1 = match.team1.clan.show;
	if (show1)
	{
		match.team1.clan.tag.UpdateLabel(this->team1Tag);
		match.team1.clan.name.UpdateLabel(this->team1Name);
		match.team1.clan.region.UpdateLabel(this->team1Region);
	}
	this->team1Tag->setVisible(show1);
	this->team1Name->setVisible(show1);
	this->team1Region->setVisible(show1);
	this->team1RegionLabel->setVisible(show1);

	bool show2 = match.team1.clan.show;
	if (show2)
	{
		match.team2.clan.tag.UpdateLabel(this->team2Tag);
		match.team2.clan.name.UpdateLabel(this->team2Name);
		match.team2.clan.region.UpdateLabel(this->team2Region);
	}
	this->team2Tag->setVisible(show2);
	this->team2Name->setVisible(show2);
	this->team2Region->setVisible(show2);
	this->team2RegionLabel->setVisible(show2);
}

void StatsTeamFooter::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->team1WrLabel->setText(GetString(StringKeys::LABEL_WINRATE));
		this->team1DmgLabel->setText(GetString(StringKeys::LABEL_DAMAGE));
		this->team1RegionLabel->setText(GetString(StringKeys::LABEL_REGION));
		this->team2WrLabel->setText(GetString(StringKeys::LABEL_WINRATE));
		this->team2DmgLabel->setText(GetString(StringKeys::LABEL_DAMAGE));
		this->team2RegionLabel->setText(GetString(StringKeys::LABEL_REGION));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}

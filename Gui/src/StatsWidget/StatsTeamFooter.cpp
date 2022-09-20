// Copyright 2020 <github.com/razaqq>

#include "Client/StatsParser.hpp"
#include "Client/StringTable.hpp"

#include "Gui/LanguageChangeEvent.hpp"
#include "Gui/StatsWidget/StatsTeamFooter.hpp"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include <array>


using namespace PotatoAlert::Client::StringTable;
using namespace PotatoAlert::Core;
using PotatoAlert::Gui::StatsTeamFooter;

StatsTeamFooter::StatsTeamFooter(QWidget* parent) : QWidget(parent)
{
	Init();
}

void StatsTeamFooter::Init()
{
	qApp->installEventFilter(this);

	auto layout = new QHBoxLayout()();
	layout->setContentsMargins(10, 0, 10, 0);
	layout->setSpacing(10);

	const QFont labelFont = QFont("Segoe UI", 10, QFont::Bold);

	// left side
	auto leftWidget = new QWidget();
	auto leftLayout = new QHBoxLayout();
	leftLayout->setContentsMargins(10, 0, 10, 0);
	leftLayout->setSpacing(20);

	// right side
	auto rightWidget = new QWidget();
	auto rightLayout = new QHBoxLayout();
	rightLayout->setContentsMargins(10, 0, 10, 0);
	rightLayout->setSpacing(20);

	// set font on all labels
	const std::array labels{
		std::array{ m_team1WrLabel, m_team1Wr, m_team1DmgLabel, m_team1Dmg, m_team1Tag, m_team1Name, m_team1RegionLabel, m_team1Region },
		std::array{ m_team2WrLabel, m_team2Wr, m_team2DmgLabel, m_team2Dmg, m_team2Tag, m_team2Name, m_team2RegionLabel, m_team2Region }
	};
	for (auto& side : labels)
		for (auto& label : side)
			label->setFont(labelFont);

	// add labels
	for (size_t side = 0; side < 2; side++)
	{
		for (size_t element = 0; element < 4; element++)
		{
			auto w = new QWidget();
			auto l = new QHBoxLayout();
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

	m_team1RegionLabel->setVisible(false);
	m_team2RegionLabel->setVisible(false);

	leftWidget->setLayout(leftLayout);
	rightWidget->setLayout(rightLayout);
	layout->addWidget(leftWidget);
	layout->addWidget(rightWidget);
	setLayout(layout);
}

void StatsTeamFooter::Update(const MatchType& match) const
{
	// set average stats per team
	match.Team1.Winrate.UpdateLabel(m_team1Wr);
	match.Team1.AvgDmg.UpdateLabel(m_team1Dmg);
	match.Team2.Winrate.UpdateLabel(m_team2Wr);
	match.Team2.AvgDmg.UpdateLabel(m_team2Dmg);

	// set clan battle stuff
	bool show1 = match.Team1.Clan.Show;
	if (show1)
	{
		match.Team1.Clan.Tag.UpdateLabel(m_team1Tag);
		match.Team1.Clan.Name.UpdateLabel(m_team1Name);
		match.Team1.Clan.Region.UpdateLabel(m_team1Region);
	}
	m_team1Tag->setVisible(show1);
	m_team1Name->setVisible(show1);
	m_team1Region->setVisible(show1);
	m_team1RegionLabel->setVisible(show1);

	bool show2 = match.Team1.Clan.Show;
	if (show2)
	{
		match.Team2.Clan.Tag.UpdateLabel(m_team2Tag);
		match.Team2.Clan.Name.UpdateLabel(m_team2Name);
		match.Team2.Clan.Region.UpdateLabel(m_team2Region);
	}
	m_team2Tag->setVisible(show2);
	m_team2Name->setVisible(show2);
	m_team2Region->setVisible(show2);
	m_team2RegionLabel->setVisible(show2);
}

bool StatsTeamFooter::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_team1WrLabel->setText(GetString(lang, StringTableKey::LABEL_WINRATE));
		m_team1DmgLabel->setText(GetString(lang, StringTableKey::LABEL_DAMAGE));
		m_team1RegionLabel->setText(GetString(lang, StringTableKey::LABEL_REGION));
		m_team2WrLabel->setText(GetString(lang, StringTableKey::LABEL_WINRATE));
		m_team2DmgLabel->setText(GetString(lang, StringTableKey::LABEL_DAMAGE));
		m_team2RegionLabel->setText(GetString(lang, StringTableKey::LABEL_REGION));
	}
	return QWidget::eventFilter(watched, event);
}

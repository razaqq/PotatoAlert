// Copyright 2020 <github.com/razaqq>

#include "Client/StatsParser.hpp"
#include "Client/StringTable.hpp"

#include "Gui/Events.hpp"
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

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(10, 0, 10, 0);
	layout->setSpacing(10);

	layout->setContentsMargins(10, 0, 10, 0);
	layout->setSpacing(0);

	layout->addWidget(m_wrLabel, 0, Qt::AlignRight);
	layout->addWidget(m_wr, 0, Qt::AlignLeft);

	layout->addSpacing(20);

	layout->addWidget(m_dmgLabel, 0, Qt::AlignRight);
	layout->addWidget(m_dmg, 0, Qt::AlignLeft);

	layout->addStretch();

	layout->addWidget(m_tag, 0, Qt::AlignRight);
	layout->addWidget(m_name, 0, Qt::AlignLeft);

	layout->addStretch();

	layout->addWidget(m_regionLabel, 0, Qt::AlignRight);
	layout->addWidget(m_region, 0, Qt::AlignLeft);

	m_regionLabel->setVisible(false);

	setLayout(layout);
}

void StatsTeamFooter::Update(const Team& team) const
{
	// set average stats per team
	team.Winrate.UpdateLabel(m_wr);
	team.AvgDmg.UpdateLabel(m_dmg);

	// set clan battle stuff
	bool show = team.Clan.Show;
	if (show)
	{
		team.Clan.Tag.UpdateLabel(m_tag);
		team.Clan.Name.UpdateLabel(m_name);
		team.Clan.Region.UpdateLabel(m_region);
	}
	m_tag->setVisible(show);
	m_name->setVisible(show);
	m_region->setVisible(show);
	m_regionLabel->setVisible(show);
}

bool StatsTeamFooter::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_wrLabel->setText(GetString(lang, StringTableKey::LABEL_WINRATE));
		m_dmgLabel->setText(GetString(lang, StringTableKey::LABEL_DAMAGE));
		m_regionLabel->setText(GetString(lang, StringTableKey::LABEL_REGION));
	}
	return QWidget::eventFilter(watched, event);
}

// Copyright 2020 <github.com/razaqq>

#include "Gui/IconButton.hpp"
#include "Gui/VerticalMenuBar.hpp"

#include <QButtonGroup>
#include <QDockWidget>
#include <QIcon>
#include <QPixmap>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>


using PotatoAlert::Gui::VerticalMenuBar;

VerticalMenuBar::VerticalMenuBar(QWidget* parent) : QDockWidget(parent)
{
	Init();
}

void VerticalMenuBar::Init()
{
	setAttribute(Qt::WA_StyledBackground, true);
	setObjectName("menuBar");

	setTitleBarWidget(new QWidget());

	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(DockWidgetMovable | DockWidgetVerticalTitleBar);
	setMinimumHeight(100);

	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 10, 0, 10);
	layout->setSpacing(0);

	setFixedWidth(30);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

	constexpr QSize iconSize = QSize(20, 20);
	IconButton* table = new IconButton(":/Table.svg", ":/TableHover.svg", iconSize, true);
	IconButton* settings = new IconButton(":/Settings.svg", ":/SettingsHover.svg", iconSize, true);
	IconButton* matchHistory = new IconButton(":/MatchHistory.svg", ":/MatchHistoryHover.svg", iconSize, true);
	IconButton* discord = new IconButton(":/Discord.svg", ":/DiscordHover.svg", iconSize);
	IconButton* screenshot = new IconButton(":/Screenshot.svg", ":/ScreenshotHover.svg", iconSize);
	IconButton* csvMatches = new IconButton(":/Csv.svg", ":/CsvHover.svg", iconSize);
	IconButton* log = new IconButton(":/Log.svg", ":/LogHover.svg", iconSize);
	IconButton* github = new IconButton(":/Github.svg", ":/GithubHover.svg", iconSize);
	IconButton* about = new IconButton(":/About.svg", ":/AboutHover.svg", iconSize, true);

	m_menuEntries = { table, settings, discord, matchHistory, screenshot, csvMatches, log, github, about };

	for (size_t i = 0; i < m_menuEntries.size(); i++)
	{
		m_menuEntries[i]->setFixedWidth(width());
		m_btnGroup->addButton(m_menuEntries[i]);
		m_btnGroup->setId(m_menuEntries[i], static_cast<int>(i));
	}
	m_btnGroup->setExclusive(true);

	SetChecked(MenuEntry::Table);

	layout->addWidget(table, 0, Qt::AlignTop | Qt::AlignHCenter);
	layout->addWidget(settings, 0, Qt::AlignTop | Qt::AlignHCenter);
	layout->addWidget(matchHistory, 0, Qt::AlignTop | Qt::AlignHCenter);
	layout->addStretch();
	layout->addWidget(discord, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(screenshot, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(csvMatches, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(log, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(github, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(about, 0, Qt::AlignBottom | Qt::AlignHCenter);

	titleBarWidget()->setLayout(layout);

	connect(m_btnGroup, &QButtonGroup::idClicked, [this](int id)
	{
		emit EntryClicked(static_cast<MenuEntry>(id));
	});
}

void VerticalMenuBar::SetChecked(MenuEntry entry) const
{
	for (auto& menuEntry : m_menuEntries)
	{
		menuEntry->setChecked(false);
	}
	m_menuEntries[static_cast<int>(entry)]->setChecked(true);
}

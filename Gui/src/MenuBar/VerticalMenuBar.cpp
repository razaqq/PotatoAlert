// Copyright 2020 <github.com/razaqq>

#include "Gui/MenuBar/MenuEntryButton.hpp"
#include "Gui/MenuBar/VerticalMenuBar.hpp"

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
	this->Init();
}

void VerticalMenuBar::Init()
{
	this->setAttribute(Qt::WA_StyledBackground, true);
	this->setObjectName("menuBar");

	this->setTitleBarWidget(new QWidget());

	this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar);
	this->setMinimumHeight(100);

	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 10, 0, 10);
	layout->setSpacing(0);

	this->setFixedWidth(30);
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

	auto table = new MenuEntryButton(this, QIcon(QPixmap(":/table.svg")));
	auto settings = new MenuEntryButton(this, QIcon(QPixmap(":/settings.svg")));
	auto matchHistory = new MenuEntryButton(this, QIcon(QPixmap(":/match_history.svg")));

	auto discord = new MenuEntryButton(this, QIcon(QPixmap(":/discord.svg")), false);
	auto screenshot = new MenuEntryButton(this, QIcon(QPixmap(":/screenshot.svg")), false);
	auto csvMatches = new MenuEntryButton(this, QIcon(QPixmap(":/csv.svg")), false);
	auto log = new MenuEntryButton(this, QIcon(QPixmap(":/log.svg")), false);
	auto github = new MenuEntryButton(this, QIcon(QPixmap(":/github.svg")), false);
	auto about = new MenuEntryButton(this, QIcon(QPixmap(":/about.svg")));

	this->m_menuEntries = { table, settings, discord, matchHistory, screenshot, csvMatches, log, github, about };

	for (size_t i = 0; i < this->m_menuEntries.size(); i++)
	{
		auto button = this->m_menuEntries[i]->GetButton();
		this->m_btnGroup->addButton(button);
		this->m_btnGroup->setId(button, static_cast<int>(i));
	}
	this->m_btnGroup->setExclusive(true);

	this->SetChecked(MenuEntry::Table);

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

	this->titleBarWidget()->setLayout(layout);

	connect(this->m_btnGroup, &QButtonGroup::idClicked, [this](int id)
	{
		emit this->EntryClicked(static_cast<MenuEntry>(id));
	});
}

void VerticalMenuBar::SetChecked(MenuEntry entry)
{
	for (auto& menuEntry : this->m_menuEntries)
	{
		menuEntry->SetChecked(false);
	}
	this->m_menuEntries[static_cast<int>(entry)]->SetChecked(true);
}

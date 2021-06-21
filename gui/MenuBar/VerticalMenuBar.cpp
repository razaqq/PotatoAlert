// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QIcon>
#include <QPixmap>
#include <QButtonGroup>
#include <QDockWidget>
#include "VerticalMenuBar.hpp"
#include "MenuEntry.hpp"


using PotatoAlert::VerticalMenuBar;

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

	auto table = new MenuEntry(this, QIcon(QPixmap(":/table.svg")));
	auto settings = new MenuEntry(this, QIcon(QPixmap(":/settings.svg")));

	auto discord = new MenuEntry(this, QIcon(QPixmap(":/discord.svg")));
	auto log = new MenuEntry(this, QIcon(QPixmap(":/log.svg")));
	auto github = new MenuEntry(this, QIcon(QPixmap(":/github.svg")));
	auto about = new MenuEntry(this, QIcon(QPixmap(":/about.svg")));

	table->button->setChecked(true);
	discord->button->setCheckable(false);
	log->button->setCheckable(false);
	github->button->setCheckable(false);

	this->btnGroup->addButton(table->button);
	this->btnGroup->setId(table->button, 0);
	this->btnGroup->addButton(settings->button);
	this->btnGroup->setId(settings->button, 1);
	this->btnGroup->addButton(discord->button);
	this->btnGroup->setId(discord->button, 2);
	this->btnGroup->addButton(log->button);
	this->btnGroup->setId(log->button, 3);
	this->btnGroup->addButton(github->button);
	this->btnGroup->setId(github->button, 4);
	this->btnGroup->addButton(about->button);
	this->btnGroup->setId(about->button, 5);
	this->btnGroup->setExclusive(true);

	layout->addWidget(table, 0, Qt::AlignTop | Qt::AlignHCenter);
	layout->addWidget(settings, 0, Qt::AlignTop | Qt::AlignHCenter);
	layout->addStretch();
	layout->addWidget(discord, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(log, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(github, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(about, 0, Qt::AlignBottom | Qt::AlignHCenter);

	this->titleBarWidget()->setLayout(layout);
}

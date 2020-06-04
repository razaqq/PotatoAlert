// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include <QButtonGroup>
#include "VerticalMenuBar.h"
#include "MenuEntry.h"


using PotatoAlert::VerticalMenuBar;

VerticalMenuBar::VerticalMenuBar(QWidget* parent) : QWidget(parent)
{
	this->init();
}

void VerticalMenuBar::init()
{
	this->setObjectName("menuBar");

	// this->setMouseTracking(true);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setContentsMargins(0, 10, 0, 10);
	layout->setSpacing(0);

	this->setFixedWidth(30);
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

	MenuEntry* table = new MenuEntry(this, QIcon(QPixmap(":/table.svg")));
	MenuEntry* settings = new MenuEntry(this, QIcon(QPixmap(":/settings.svg")));

	MenuEntry* discord = new MenuEntry(this, QIcon(QPixmap(":/discord.svg")));
	MenuEntry* help = new MenuEntry(this, QIcon(QPixmap(":/help.svg")));
	MenuEntry* log = new MenuEntry(this, QIcon(QPixmap(":/log.svg")));
	MenuEntry* github = new MenuEntry(this, QIcon(QPixmap(":/github.svg")));
	MenuEntry* about = new MenuEntry(this, QIcon(QPixmap(":/about.svg")));

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
	this->btnGroup->addButton(help->button);
	this->btnGroup->setId(help->button, 3);
	this->btnGroup->addButton(log->button);
	this->btnGroup->setId(log->button, 4);
	this->btnGroup->addButton(github->button);
	this->btnGroup->setId(github->button, 5);
	this->btnGroup->addButton(about->button);
	this->btnGroup->setId(about->button, 6);
	this->btnGroup->setExclusive(true);

	layout->addWidget(table, 0, Qt::AlignTop | Qt::AlignHCenter);
	layout->addWidget(settings, 0, Qt::AlignTop | Qt::AlignHCenter);
	layout->addStretch();
	layout->addWidget(discord, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(help, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(log, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(github, 0, Qt::AlignBottom | Qt::AlignHCenter);
	layout->addWidget(about, 0, Qt::AlignBottom | Qt::AlignHCenter);

	this->setLayout(layout);
}

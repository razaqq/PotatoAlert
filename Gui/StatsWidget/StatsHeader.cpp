// Copyright 2020 <github.com/razaqq>

#include "StatsHeader.hpp"

#include "Config.hpp"
#include "PotatoClient.hpp"
#include "StringTable.hpp"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QSize>
#include <QWidget>

#include <string>


using PotatoAlert::Gui::StatsHeader;

StatsHeader::StatsHeader(QWidget* parent) : QWidget(parent)
{
	this->Init();
}

void StatsHeader::Init()
{
	this->m_loading->setSpeed(1000);
	this->m_loading->setScaledSize(QSize(20, 20));
	this->m_ready = this->m_ready.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	this->m_error = this->m_error.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	auto layout = new QHBoxLayout();
	layout->setContentsMargins(10, 0, 10, 0);
	layout->setSpacing(10);

	auto leftLayout = new QHBoxLayout();
	auto rightLayout = new QHBoxLayout();

	const QFont labelFont("Segoe UI", 16, QFont::Bold);

	// status icon and text
	auto status = new QWidget(this);
	auto statusLayout = new QHBoxLayout();
	statusLayout->setContentsMargins(0, 0, 0, 0);
	statusLayout->setSpacing(0);
	status->setFixedWidth(130);
	this->m_statusIcon->setFixedSize(20, 20);
	statusLayout->addWidget(this->m_statusIcon);
	statusLayout->addSpacing(5);
	this->m_statusText->setAlignment(Qt::AlignCenter);
	this->m_statusText->setStyleSheet("font-size: 10px;");
	statusLayout->addWidget(this->m_statusText);
	statusLayout->addStretch();
	status->setLayout(statusLayout);

	// team labels
	this->m_team1Label->setFont(labelFont);
	this->m_team2Label->setFont(labelFont);

	// dummy with same width as status
	auto dummy = new QWidget();
	dummy->setFixedWidth(130);

	// add to layouts
	leftLayout->addWidget(status);
	leftLayout->addStretch();
	leftLayout->addWidget(this->m_team1Label);
	leftLayout->addStretch();
	leftLayout->addWidget(dummy);

	rightLayout->addStretch();
	rightLayout->addWidget(this->m_team2Label);
	rightLayout->addStretch();

	layout->addLayout(leftLayout);
	layout->addLayout(rightLayout);
	this->setLayout(layout);
}

void StatsHeader::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->m_team1Label->setText(GetString(PotatoAlert::StringTable::Keys::LABEL_MYTEAM));
		this->m_team2Label->setText(GetString(PotatoAlert::StringTable::Keys::LABEL_ENEMYTEAM));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}

void StatsHeader::SetStatus(Status status, const std::string& text)
{
	this->m_statusText->setText(QString::fromStdString(text));
	this->m_statusIcon->clear();

	switch (status)
	{
	case Status::Ready:
		m_loading->stop();
		this->m_statusIcon->setPixmap(this->m_ready);
		break;
	case Status::Loading:
		this->m_statusIcon->setMovie(this->m_loading);
		m_loading->start();
		break;
	case Status::Error:
		m_loading->stop();
		this->m_statusIcon->setPixmap(this->m_error);
		break;
	}
}

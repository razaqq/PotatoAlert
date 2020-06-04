// Copyright 2020 <github.com/razaqq>

#include "StatsHeader.h"
#include <atlstr.h>
#include <QWidget>
#include <QLabel>
#include <QFont>
#include <QSize>
#include <string>


using PotatoAlert::StatsHeader;

// statuses
#define STATUS_READY 0
#define STATUS_LOADING 1
#define STATUS_ERROR 2

StatsHeader::StatsHeader(QWidget* parent) : QWidget(parent)
{
	this->init();
}

void StatsHeader::init()
{
	this->loading->setSpeed(1000);
	this->loading->setScaledSize(QSize(20, 20));
	this->ready = this->ready.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	this->error = this->error.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(10, 0, 10, 0);
	layout->setSpacing(10);

	QHBoxLayout* leftLayout = new QHBoxLayout;
	QHBoxLayout* rightLayout = new QHBoxLayout;

	const QFont labelFont("Segoe UI", 16, QFont::Bold);

	// status icon and text
	QWidget* status = new QWidget(this);
	QHBoxLayout* statusLayout = new QHBoxLayout;
	statusLayout->getContentsMargins(0, 0, 0, 0);
	statusLayout->setSpacing(0);
	status->setFixedWidth(130);
	this->statusIcon->setFixedSize(20, 20);
	statusLayout->addWidget(this->statusIcon);
	statusLayout->addSpacing(5);
	this->statusText->setAlignment(Qt::AlignCenter);
	this->statusText->setStyleSheet("font-size: 10px;");
	statusLayout->addWidget(this->statusText);
	statusLayout->addStretch();
	status->setLayout(statusLayout);

	// team 1 label
	CStringW text;
	text.LoadStringA(104);
	QLabel* team1 = new QLabel(QString::fromWCharArray(text, text.GetLength()));
	team1->setFont(labelFont);

	// team 2 label
	text.LoadStringA(105);
	QLabel* team2 = new QLabel(QString::fromWCharArray(text, text.GetLength()));
	team2->setFont(labelFont);

	// dummy with same width as status
	QWidget* dummy = new QWidget;
	dummy->setFixedWidth(130);

	// add to layouts
	leftLayout->addWidget(status);
	leftLayout->addStretch();
	leftLayout->addWidget(team1);
	leftLayout->addStretch();
	leftLayout->addWidget(dummy);

	rightLayout->addStretch();
	rightLayout->addWidget(team2);
	rightLayout->addStretch();

	layout->addLayout(leftLayout);
	layout->addLayout(rightLayout);
	this->setLayout(layout);
}


void StatsHeader::setStatus(int statusID, const std::string& text)
{
	this->statusText->setText(QString::fromStdString(text));
	this->statusIcon->clear();

	switch (statusID)
	{
	case STATUS_READY:
		loading->stop();
		this->statusIcon->setPixmap(this->ready);
		break;
	case STATUS_LOADING:
		this->statusIcon->setMovie(this->loading);
		loading->start();
		break;
	case STATUS_ERROR:
		loading->stop();
		this->statusIcon->setPixmap(this->error);
		break;
	default:
		break;
	}
}

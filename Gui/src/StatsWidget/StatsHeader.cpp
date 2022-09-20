// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/StringTable.hpp"

#include "Gui/LanguageChangeEvent.hpp"
#include "Gui/StatsWidget/StatsHeader.hpp"

#include <QApplication>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QSize>
#include <QSvgWidget>
#include <QWidget>
#include <QWindow>

#include <string>


using namespace PotatoAlert::Client::StringTable;
using PotatoAlert::Gui::StatsHeader;

StatsHeader::StatsHeader(QWidget* parent) : QWidget(parent)
{
	Init();
}

void StatsHeader::Init()
{
	qApp->installEventFilter(this);

	m_loading = new QSvgWidget(":/Loading.svg");
	m_ready = QIcon(":/Ready.svg").pixmap(QSize(20, 20), window()->devicePixelRatio());
	m_error = QIcon(":/Error.svg").pixmap(QSize(20, 20), window()->devicePixelRatio());
	
	QHBoxLayout* iconLayout = new QHBoxLayout();
	iconLayout->setContentsMargins(0, 0, 0, 0);
	iconLayout->addWidget(m_loading);
	m_statusIcon->setLayout(iconLayout);

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
	m_statusIcon->setFixedSize(20, 20);
	statusLayout->addWidget(m_statusIcon);
	statusLayout->addSpacing(5);
	m_statusText->setAlignment(Qt::AlignCenter);
	m_statusText->setStyleSheet("font-size: 10px;");
	statusLayout->addWidget(m_statusText);
	statusLayout->addStretch();
	status->setLayout(statusLayout);

	// team labels
	m_team1Label->setFont(labelFont);
	m_team2Label->setFont(labelFont);

	// dummy with same width as status
	auto dummy = new QWidget();
	dummy->setFixedWidth(130);

	// add to layouts
	leftLayout->addWidget(status);
	leftLayout->addStretch();
	leftLayout->addWidget(m_team1Label);
	leftLayout->addStretch();
	leftLayout->addWidget(dummy);

	rightLayout->addStretch();
	rightLayout->addWidget(m_team2Label);
	rightLayout->addStretch();

	layout->addLayout(leftLayout);
	layout->addLayout(rightLayout);
	setLayout(layout);
}

bool StatsHeader::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_team1Label->setText(GetString(lang, StringTableKey::LABEL_MYTEAM));
		m_team2Label->setText(GetString(lang, StringTableKey::LABEL_ENEMYTEAM));
	}
	return QWidget::eventFilter(watched, event);
}

void StatsHeader::SetStatus(Status status, std::string_view text) const
{
	m_statusText->setText(text.data());
	m_loading->setVisible(false);
	m_statusIcon->clear();

	switch (status)
	{
		case Status::Ready:
		{
			m_statusIcon->setPixmap(m_ready);
			break;
		}
		case Status::Loading:
		{
			m_loading->setVisible(true);
			break;
		}
		case Status::Error:
		{
			m_statusIcon->setPixmap(m_error);
			break;
		}
	}
}

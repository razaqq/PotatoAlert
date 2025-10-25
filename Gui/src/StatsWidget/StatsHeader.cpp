// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/StringTable.hpp"

#include "Gui/Events.hpp"
#include "Gui/StatsWidget/StatsHeader.hpp"

#include <QApplication>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QScreen>
#include <QSize>
#include <QSvgWidget>
#include <QWidget>
#include <QWindow>


using namespace PotatoAlert::Client::StringTable;
using PotatoAlert::Gui::StatsHeaderEnemy;
using PotatoAlert::Gui::StatsHeaderFriendly;

StatsHeaderFriendly::StatsHeaderFriendly(QWidget* parent) : QWidget(parent)
{
	Init();
}

void StatsHeaderFriendly::Init()
{
	static constexpr QSize iconSize(20, 20);

	qApp->installEventFilter(this);

	m_refresh = new IconButton(":/Refresh.svg", ":/RefreshHover.svg", iconSize, false);
	m_loading = new QSvgWidget(":/Loading.svg");
	m_ready = QIcon(":/Ready.svg").pixmap(iconSize, window()->devicePixelRatio());
	m_error = QIcon(":/Error.svg").pixmap(iconSize, window()->devicePixelRatio());

	connect(m_refresh, &IconButton::clicked, [this]([[maybe_unused]] bool checked)
	{
		emit ForceRefresh();
	});
	
	QHBoxLayout* iconLayout = new QHBoxLayout();
	iconLayout->setContentsMargins(0, 0, 0, 0);
	iconLayout->addWidget(m_loading);
	m_statusIcon->setLayout(iconLayout);

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(10, 0, 10, 2);
	layout->setSpacing(10);

	// status icon and text
	static constexpr int totalWidth = 155;

	QWidget* status = new QWidget(this);
	QHBoxLayout* statusLayout = new QHBoxLayout();
	statusLayout->setContentsMargins(0, 0, 0, 0);
	statusLayout->setSpacing(0);
	status->setFixedWidth(totalWidth);
	m_statusIcon->setFixedSize(iconSize);
	statusLayout->addWidget(m_statusIcon);          // 20
	statusLayout->addSpacing(5);               // 5
	m_statusText->setAlignment(Qt::AlignCenter);
	statusLayout->addWidget(m_statusText);
	statusLayout->addSpacing(5);               // 5
	statusLayout->addWidget(m_refresh);             // 20
	statusLayout->addStretch();
	status->setLayout(statusLayout);

	// dummy with same width as status
	QWidget* dummy = new QWidget();
	dummy->setFixedWidth(totalWidth);

	// add to layouts
	layout->addWidget(status);
	layout->addStretch();
	layout->addWidget(m_label, 0, Qt::AlignCenter);
	layout->addStretch();
	layout->addWidget(dummy);

	setLayout(layout);
}

bool StatsHeaderFriendly::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_label->setText(GetString(lang, StringTableKey::LABEL_MYTEAM));
	}
	return QWidget::eventFilter(watched, event);
}

void StatsHeaderFriendly::SetStatus(Client::Status status, std::string_view text) const
{
	m_statusText->setText(text.data());
	m_loading->setVisible(false);
	m_statusIcon->clear();

	switch (status)
	{
		case Client::Status::Ready:
		{
			m_refresh->setVisible(false);
			m_statusIcon->setPixmap(m_ready);
			break;
		}
		case Client::Status::Loading:
		{
			m_refresh->setVisible(false);
			m_loading->setVisible(true);
			break;
		}
		case Client::Status::Error:
		{
			m_refresh->setVisible(true);
			m_statusIcon->setPixmap(m_error);
			break;
		}
	}
}

StatsHeaderEnemy::StatsHeaderEnemy(QWidget* parent) : QWidget(parent)
{
	Init();
}

void StatsHeaderEnemy::Init()
{
	qApp->installEventFilter(this);

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(10, 0, 10, 2);
	layout->setSpacing(10);

	layout->addStretch();
	layout->addWidget(m_label, 0, Qt::AlignCenter);
	layout->addStretch();

	setLayout(layout);
}

bool StatsHeaderEnemy::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_label->setText(GetString(lang, StringTableKey::LABEL_ENEMYTEAM));
	}
	return QWidget::eventFilter(watched, event);
}

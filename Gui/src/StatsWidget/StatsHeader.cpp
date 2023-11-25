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


using namespace PotatoAlert::Client::StringTable;
using PotatoAlert::Gui::StatsHeaderEnemy;
using PotatoAlert::Gui::StatsHeaderFriendly;

StatsHeaderFriendly::StatsHeaderFriendly(QWidget* parent) : QWidget(parent)
{
	Init();
}

void StatsHeaderFriendly::Init()
{
	qApp->installEventFilter(this);

	m_loading = new QSvgWidget(":/Loading.svg");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	m_ready = QIcon(":/Ready.svg").pixmap(QSize(20, 20), window()->devicePixelRatio());
	m_error = QIcon(":/Error.svg").pixmap(QSize(20, 20), window()->devicePixelRatio());
#else
	m_ready = QIcon(":/Ready.svg").pixmap(QSize(20, 20));
	m_error = QIcon(":/Error.svg").pixmap(QSize(20, 20));
#endif
	
	QHBoxLayout* iconLayout = new QHBoxLayout();
	iconLayout->setContentsMargins(0, 0, 0, 0);
	iconLayout->addWidget(m_loading);
	m_statusIcon->setLayout(iconLayout);

	auto layout = new QHBoxLayout();
	layout->setContentsMargins(10, 0, 10, 2);
	layout->setSpacing(10);

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

	m_label->setFont(QFont(QApplication::font().family(), 15, QFont::Bold));

	// dummy with same width as status
	QWidget* dummy = new QWidget();
	dummy->setFixedWidth(130);

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
			m_statusIcon->setPixmap(m_ready);
			break;
		}
		case Client::Status::Loading:
		{
			m_loading->setVisible(true);
			break;
		}
		case Client::Status::Error:
		{
			m_statusIcon->setPixmap(m_error);
			break;
		}
	}
}

StatsHeaderEnemy::StatsHeaderEnemy(QWidget* parent)
{
	Init();
}

void StatsHeaderEnemy::Init()
{
	qApp->installEventFilter(this);

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(10, 0, 10, 2);
	layout->setSpacing(10);

	m_label->setFont(QFont(QApplication::font().family(), 15, QFont::Bold));

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

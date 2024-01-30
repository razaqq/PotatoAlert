// Copyright 2020 <github.com/razaqq>

#include "Gui/TitleBar.hpp"
#include "Gui/Fonts.hpp"

#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QObject>
#include <QPixmap>
#include <QSizePolicy>
#include <QToolButton>
#include <QWidget>


using PotatoAlert::Gui::TitleBar;

TitleBar::TitleBar(QWidget* parent) : QWidget(parent), m_parentWindow(parent)
{
	Init();
}

void TitleBar::Init()
{
	setAttribute(Qt::WA_StyledBackground, true);
	setObjectName("titleBar");

	QHBoxLayout* hLayout = new QHBoxLayout();
	hLayout->setContentsMargins(5, 0, 0, 0);
	hLayout->setSpacing(0);

	m_parentWindow->installEventFilter(this);

	m_appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	constexpr QSizePolicy buttonPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_btnMinimize->setSizePolicy(buttonPolicy);
	m_btnMaximize->setSizePolicy(buttonPolicy);
	m_btnClose->setSizePolicy(buttonPolicy);

	m_btnMinimize->setIcon(QIcon(QPixmap(":/Minimize.svg").scaledToHeight(20, Qt::SmoothTransformation)));
	m_btnMaximize->setIcon(QIcon(QPixmap(":/Maximize.svg").scaledToHeight(20, Qt::SmoothTransformation)));
	m_btnClose->setIcon(QIcon(QPixmap(":/Close.svg").scaledToHeight(20, Qt::SmoothTransformation)));

	m_appName->setText("PotatoAlert");
	m_appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_appIcon->setIcon(QIcon(":/potato.svg"));
	m_appIcon->setIconSize(QSize(12, 12));

	m_appIcon->setObjectName("appIcon");
	m_btnMinimize->setObjectName("btnMinimize");
	m_btnMaximize->setObjectName("btnMaximize");
	m_btnClose->setObjectName("btnClose");

	connect(m_btnClose, &QToolButton::clicked, m_parentWindow, &QWidget::hide);
	connect(m_btnMinimize, &QToolButton::clicked, [this](bool _)
	{
		m_parentWindow->showMinimized();
	});
	connect(m_btnMaximize, &QToolButton::clicked, [this](bool _)
	{
		if (m_parentWindow->windowState().testFlag(Qt::WindowMaximized))
		{
			m_parentWindow->showNormal();
		}
		else
		{
			m_parentWindow->showMaximized();
		}
	});

	hLayout->addWidget(m_appIcon);
	hLayout->addSpacing(5);
	hLayout->addWidget(m_appName);
	hLayout->addStretch();
	hLayout->addWidget(m_btnMinimize);
	hLayout->addWidget(m_btnMaximize);
	hLayout->addWidget(m_btnClose);

	setLayout(hLayout);
}

bool TitleBar::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		if (m_parentWindow->windowState().testFlag(Qt::WindowMaximized))
		{
			m_btnMaximize->setIcon(QIcon(QPixmap(":/Restore.svg").scaledToHeight(20, Qt::SmoothTransformation)));
		}
		else
		{
			m_btnMaximize->setIcon(QIcon(QPixmap(":/Maximize.svg").scaledToHeight(20, Qt::SmoothTransformation)));
		}

		m_btnMaximize->setAttribute(Qt::WA_UnderMouse, false);
	}
	else if (event->type() == QEvent::ApplicationFontChange)
	{
		UpdateLayoutFont(layout());
	}
	QWidget::eventFilter(object, event);
	return false;
}

void TitleBar::mouseDoubleClickEvent([[maybe_unused]] QMouseEvent* event)
{
	m_btnMaximize->click();
}

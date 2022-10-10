// Copyright 2020 <github.com/razaqq>

#include "Gui/TitleBar.hpp"

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

	auto hLayout = new QHBoxLayout();
	hLayout->setContentsMargins(5, 0, 0, 0);
	hLayout->setSpacing(0);

	m_parentWindow->installEventFilter(this);

	m_appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QSizePolicy buttonPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_btnMinimize->setSizePolicy(buttonPolicy);
	m_btnMaximize->setSizePolicy(buttonPolicy);
	m_btnRestore->setSizePolicy(buttonPolicy);
	m_btnClose->setSizePolicy(buttonPolicy);

	m_btnMinimize->setIcon(QIcon(QPixmap(":/Minimize.svg").scaledToHeight(20, Qt::SmoothTransformation)));
	m_btnMaximize->setIcon(QIcon(QPixmap(":/Maximize.svg").scaledToHeight(20, Qt::SmoothTransformation)));
	m_btnRestore->setIcon(QIcon(QPixmap(":/Restore.svg").scaledToHeight(20, Qt::SmoothTransformation)));
	m_btnClose->setIcon(QIcon(QPixmap(":/Close.svg").scaledToHeight(20, Qt::SmoothTransformation)));

	m_appName->setText("PotatoAlert");
	m_appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_appIcon->setIcon(QIcon(":/potato.png"));
	m_appIcon->setIconSize(QSize(12, 12));

	m_appIcon->setObjectName("appIcon");
	m_btnMinimize->setObjectName("btnMinimize");
	m_btnMaximize->setObjectName("btnMaximize");
	m_btnRestore->setObjectName("btnRestore");
	m_btnClose->setObjectName("btnClose");

	m_btnRestore->setVisible(false);

	connect(m_btnClose, &QToolButton::clicked, m_parentWindow, &QWidget::hide);
	connect(m_btnMinimize, &QToolButton::clicked, this, &TitleBar::OnBtnMinimizeClicked);
	connect(m_btnMaximize, &QToolButton::clicked, this, &TitleBar::OnBtnMaximizeClicked);
	connect(m_btnRestore, &QToolButton::clicked, this, &TitleBar::OnBtnRestoreClicked);

	hLayout->addWidget(m_appIcon);
	hLayout->addSpacing(5);
	hLayout->addWidget(m_appName);
	hLayout->addStretch();
	hLayout->addWidget(m_btnMinimize);
	hLayout->addWidget(m_btnMaximize);
	hLayout->addWidget(m_btnRestore);
	hLayout->addWidget(m_btnClose);

	setLayout(hLayout);
}

bool TitleBar::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		if (m_parentWindow->windowState() == Qt::WindowMaximized)
		{
			m_btnMaximize->setVisible(false);
			m_btnRestore->setVisible(true);
		}
		else
		{
			m_btnMaximize->setVisible(true);
			m_btnRestore->setVisible(false);
		}

		m_btnMaximize->setAttribute(Qt::WA_UnderMouse, false);
		m_btnRestore->setAttribute(Qt::WA_UnderMouse, false);
	}
	QWidget::eventFilter(object, event);
	return false;
}

void TitleBar::OnBtnMinimizeClicked() const
{
	m_parentWindow->showMinimized();
}

void TitleBar::OnBtnMaximizeClicked() const
{
	m_parentWindow->showMaximized();
	m_btnMaximize->setVisible(false);
	m_btnRestore->setVisible(true);
}

void TitleBar::OnBtnRestoreClicked() const
{
	m_parentWindow->showNormal();
	m_btnMaximize->setVisible(true);
	m_btnRestore->setVisible(false);
}

void TitleBar::mouseDoubleClickEvent([[maybe_unused]] QMouseEvent* event)
{
	if (m_parentWindow->isMaximized())
		OnBtnRestoreClicked();
	else
		OnBtnMaximizeClicked();
}

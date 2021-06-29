// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QEvent>
#include <QObject>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QToolButton>
#include <QMouseEvent>
#include "TitleBar.hpp"


using PotatoAlert::TitleBar;

TitleBar::TitleBar(QWidget* parent) : QWidget(parent)
{
	this->m_parentWindow = parent;
	this->Init();
}

void TitleBar::Init()
{
	this->setAttribute(Qt::WA_StyledBackground, true);
	this->setObjectName("titleBar");

	auto hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(5, 0, 0, 0);
	hLayout->setSpacing(0);

	this->m_parentWindow->installEventFilter(this);

	this->m_appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	this->m_appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QSizePolicy buttonPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->m_btnMinimize->setSizePolicy(buttonPolicy);
	this->m_btnMaximize->setSizePolicy(buttonPolicy);
	this->m_btnRestore->setSizePolicy(buttonPolicy);
	this->m_btnClose->setSizePolicy(buttonPolicy);

	this->m_btnMinimize->setIcon(QIcon(QPixmap(":/minimize.svg")));
	this->m_btnMaximize->setIcon(QIcon(QPixmap(":/maximize.svg")));
	this->m_btnRestore->setIcon(QIcon(QPixmap(":/restore.svg")));
	this->m_btnClose->setIcon(QIcon(QPixmap(":/close.svg")));

	this->m_appName->setText("PotatoAlert");
	this->m_appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	this->m_appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	this->m_appIcon->setIcon(QIcon(":/potato.png"));
	this->m_appIcon->setIconSize(QSize(12, 12));

	this->m_appIcon->setObjectName("appIcon");
	this->m_btnMinimize->setObjectName("btnMinimize");
	this->m_btnMaximize->setObjectName("btnMaximize");
	this->m_btnRestore->setObjectName("btnRestore");
	this->m_btnClose->setObjectName("btnClose");

	this->m_btnRestore->setVisible(false);

	connect(this->m_btnClose, &QToolButton::clicked, this->m_parentWindow, &QWidget::close);
	connect(this->m_btnMinimize, &QToolButton::clicked, this, &TitleBar::OnBtnMinimizeClicked);
	connect(this->m_btnMaximize, &QToolButton::clicked, this, &TitleBar::OnBtnMaximizeClicked);
	connect(this->m_btnRestore, &QToolButton::clicked, this, &TitleBar::OnBtnRestoreClicked);

	hLayout->addWidget(this->m_appIcon);
	hLayout->addSpacing(5);
	hLayout->addWidget(this->m_appName);
	hLayout->addStretch();
	hLayout->addWidget(this->m_btnMinimize);
	hLayout->addWidget(this->m_btnMaximize);
	hLayout->addWidget(this->m_btnRestore);
	hLayout->addWidget(this->m_btnClose);

	this->setLayout(hLayout);
}

bool TitleBar::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		if (this->m_parentWindow->windowState() == Qt::WindowMaximized)
		{
			this->m_btnMaximize->setVisible(false);
			this->m_btnRestore->setVisible(true);
		}
		else
		{
			this->m_btnMaximize->setVisible(true);
			this->m_btnRestore->setVisible(false);
		}

		this->m_btnMaximize->setAttribute(Qt::WA_UnderMouse, false);
		this->m_btnRestore->setAttribute(Qt::WA_UnderMouse, false);
	}
	QWidget::eventFilter(object, event);
	return false;
}

void TitleBar::OnBtnMinimizeClicked()
{
	this->m_parentWindow->setWindowState(Qt::WindowMinimized);
}

void TitleBar::OnBtnMaximizeClicked()
{
	this->m_parentWindow->setWindowState(Qt::WindowMaximized);
	this->m_btnMaximize->setVisible(false);
	this->m_btnRestore->setVisible(true);
}

void TitleBar::OnBtnRestoreClicked()
{
	this->m_parentWindow->setWindowState(Qt::WindowNoState);
	this->m_btnMaximize->setVisible(true);
	this->m_btnRestore->setVisible(false);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent*)
{
	if (this->m_parentWindow->windowState() == Qt::WindowMaximized)
		this->OnBtnRestoreClicked();
	else
		this->OnBtnMaximizeClicked();
}

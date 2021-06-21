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
	this->parentWindow = parent;
	this->Init();
}

void TitleBar::Init()
{
	this->setAttribute(Qt::WA_StyledBackground, true);
	this->setObjectName("titleBar");

	auto hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(5, 0, 0, 0);
	hLayout->setSpacing(0);

	this->parentWindow->installEventFilter(this);

	this->appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	this->appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QSizePolicy buttonPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->btnMinimize->setSizePolicy(buttonPolicy);
	this->btnMaximize->setSizePolicy(buttonPolicy);
	this->btnRestore->setSizePolicy(buttonPolicy);
	this->btnClose->setSizePolicy(buttonPolicy);

	this->btnMinimize->setIcon(QIcon(QPixmap(":/minimize.svg")));
	this->btnMaximize->setIcon(QIcon(QPixmap(":/maximize.svg")));
	this->btnRestore->setIcon(QIcon(QPixmap(":/restore.svg")));
	this->btnClose->setIcon(QIcon(QPixmap(":/close.svg")));

	this->appName->setText("PotatoAlert");
	this->appName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	this->appName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	this->appIcon->setIcon(QIcon(":/potato.png"));
	this->appIcon->setIconSize(QSize(12, 12));

	this->appIcon->setObjectName("appIcon");
	this->btnMinimize->setObjectName("btnMinimize");
	this->btnMaximize->setObjectName("btnMaximize");
	this->btnRestore->setObjectName("btnRestore");
	this->btnClose->setObjectName("btnClose");

	this->btnRestore->setVisible(false);

	connect(this->btnClose, &QToolButton::clicked, this->parentWindow, &QWidget::close);
	connect(this->btnMinimize, &QToolButton::clicked, this, &TitleBar::OnBtnMinimizeClicked);
	connect(this->btnMaximize, &QToolButton::clicked, this, &TitleBar::OnBtnMaximizeClicked);
	connect(this->btnRestore, &QToolButton::clicked, this, &TitleBar::OnBtnRestoreClicked);

	hLayout->addWidget(this->appIcon);
	hLayout->addSpacing(5);
	hLayout->addWidget(this->appName);
	hLayout->addStretch();
	hLayout->addWidget(this->btnMinimize);
	hLayout->addWidget(this->btnMaximize);
	hLayout->addWidget(this->btnRestore);
	hLayout->addWidget(this->btnClose);

	this->setLayout(hLayout);
}

bool TitleBar::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		if (this->parentWindow->windowState() == Qt::WindowMaximized)
		{
			this->btnMaximize->setVisible(false);
			this->btnRestore->setVisible(true);
		}
		else
		{
			this->btnMaximize->setVisible(true);
			this->btnRestore->setVisible(false);
		}

		this->btnMaximize->setAttribute(Qt::WA_UnderMouse, false);
		this->btnRestore->setAttribute(Qt::WA_UnderMouse, false);
	}
	QWidget::eventFilter(object, event);
	return false;
}

void TitleBar::OnBtnMinimizeClicked()
{
	this->parentWindow->setWindowState(Qt::WindowMinimized);
}

void TitleBar::OnBtnMaximizeClicked()
{
	this->parentWindow->setWindowState(Qt::WindowMaximized);
	this->btnMaximize->setVisible(false);
	this->btnRestore->setVisible(true);
}

void TitleBar::OnBtnRestoreClicked()
{
	this->parentWindow->setWindowState(Qt::WindowNoState);
	this->btnMaximize->setVisible(true);
	this->btnRestore->setVisible(false);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent*)
{
	if (this->parentWindow->windowState() == Qt::WindowMaximized)
		this->OnBtnRestoreClicked();
	else
		this->OnBtnMaximizeClicked();
}

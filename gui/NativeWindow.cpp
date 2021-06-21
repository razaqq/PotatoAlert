// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QWindow>

#include "NativeWindow.hpp"
#include "TitleBar.hpp"
#include "Config.hpp"
#include "FramelessWindowsManager.hpp"


using PotatoAlert::NativeWindow;

NativeWindow::NativeWindow(QMainWindow* mainWindow) : QWidget()
{
	this->mainWindow = mainWindow;
	this->mainWindow->setParent(this);
	this->Init();
}

void NativeWindow::closeEvent(QCloseEvent* event)
{
	PotatoConfig().Set<int>("window_height", this->height());
	PotatoConfig().Set<int>("window_width", this->width());
	PotatoConfig().Set<int>("window_x", this->x());
	PotatoConfig().Set<int>("window_y", this->y());
	QWidget::closeEvent(event);
}

void NativeWindow::Init()
{
	this->createWinId();
	QWindow* w = this->windowHandle();

	this->titleBar->setFixedHeight(23);

	for (auto& o : this->titleBar->GetIgnores())
		FramelessWindowsManager::addIgnoreObject(w, o);

	FramelessWindowsManager::addWindow(w);
	FramelessWindowsManager::setBorderWidth(w, borderWidth);
	FramelessWindowsManager::setBorderHeight(w, borderWidth);
	FramelessWindowsManager::setTitleBarHeight(w, this->titleBar->height());

	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(this->titleBar);
	layout->addWidget(this->mainWindow);

	this->setLayout(layout);

	this->resize(PotatoConfig().Get<int>("window_width"), PotatoConfig().Get<int>("window_height"));
	this->move(PotatoConfig().Get<int>("window_x"), PotatoConfig().Get<int>("window_y"));
}

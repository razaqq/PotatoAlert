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
	this->init();
}

void NativeWindow::closeEvent(QCloseEvent* event)
{
	PotatoConfig().set<int>("window_height", this->height());
	PotatoConfig().set<int>("window_width", this->width());
	PotatoConfig().set<int>("window_x", this->x());
	PotatoConfig().set<int>("window_y", this->y());
	QWidget::closeEvent(event);
}

void NativeWindow::init()
{
	this->createWinId();
	QWindow* w = this->windowHandle();

	this->titleBar->setFixedHeight(23);

	for (auto& o : this->titleBar->getIgnores())
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

	this->resize(PotatoConfig().get<int>("window_width"), PotatoConfig().get<int>("window_height"));
	this->move(PotatoConfig().get<int>("window_x"), PotatoConfig().get<int>("window_y"));
}

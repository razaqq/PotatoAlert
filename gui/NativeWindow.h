// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QEvent>
#include <QByteArray>
#include <QPaintEvent>
#include "../utils/Config.h"
#include "TitleBar.h"


namespace PotatoAlert {

class NativeWindow : public QWidget
{
	Q_OBJECT
public:
	NativeWindow(QMainWindow* mainWindow, Config* c);
	bool confirmUpdate();
private:
	Config* c;
	QMainWindow* mainWindow;

	int borderWidth = 4;

	TitleBar* titleBar = new TitleBar(this);

	void closeEvent(QCloseEvent* event);
	void showEvent(QShowEvent* event);
	bool nativeEvent(const QByteArray& eventType, void* message, long* result);
	void changeEvent(QEvent* event);

	bool handleMousePressEvent(QMouseEvent* event);
	void handleMouseMoveEvent(QMouseEvent* event);

	bool eventFilter(QObject* object, QEvent* event);

	void init();
	Qt::Edges mouseLocation(QMouseEvent* event);
};

}; // namespace PotatoAlert

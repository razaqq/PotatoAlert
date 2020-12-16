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
#include "Config.hpp"
#include "TitleBar.hpp"


namespace PotatoAlert {

class NativeWindow : public QWidget
{
	Q_OBJECT
public:
	explicit NativeWindow(QMainWindow* mainWindow);
private:
	QMainWindow* mainWindow;

	static const int borderWidth = 4;

	TitleBar* titleBar = new TitleBar(this);

	void closeEvent(QCloseEvent* event) override;
	void showEvent(QShowEvent* event) override;
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
	void changeEvent(QEvent* event) override;

	bool handleMousePressEvent(QMouseEvent* event);
	void handleMouseMoveEvent(QMouseEvent* event);

	bool eventFilter(QObject* object, QEvent* event) override;

	void init();
	Qt::Edges mouseLocation(QMouseEvent* event);
};

}  // namespace PotatoAlert

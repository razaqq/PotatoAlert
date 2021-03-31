// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QMainWindow>
#include "Config.hpp"
#include "TitleBar.hpp"


namespace PotatoAlert {

class NativeWindow : public QWidget
{
	Q_OBJECT
public:
	explicit NativeWindow(QMainWindow* mainWindow);
private:
	void init();
	QMainWindow* mainWindow;

	static const int borderWidth = 4;

	TitleBar* titleBar = new TitleBar(this);

	void closeEvent(QCloseEvent* event) override;
};

}  // namespace PotatoAlert

// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Config.hpp"
#include "TitleBar.hpp"

#include <QMainWindow>
#include <QWidget>


namespace PotatoAlert::Gui {

class NativeWindow : public QWidget
{
	Q_OBJECT
public:
	explicit NativeWindow(QMainWindow* mainWindow);
private:
	void Init();
	QMainWindow* m_mainWindow;

	static const int m_borderWidth = 4;

	TitleBar* m_titleBar = new TitleBar(this);

	void closeEvent(QCloseEvent* event) override;
};

}  // namespace PotatoAlert::Gui

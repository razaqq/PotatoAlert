// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Config.hpp"

#include "FramelessWidget"

#include "TitleBar.hpp"

#include <QMainWindow>
#include <QWidget>


namespace PotatoAlert::Gui {

class NativeWindow : public QWidget
{
	Q_OBJECT

public:
	explicit NativeWindow(QMainWindow* mainWindow, QWidget* parent = nullptr);
	static void RequestFocus();

private:
	QMainWindow* m_mainWindow;
	TitleBar* m_titleBar = new TitleBar(this);

	void Init();

	void closeEvent(QCloseEvent* event) override;
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
};

}  // namespace PotatoAlert::Gui

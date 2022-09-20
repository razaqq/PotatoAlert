// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Config.hpp"

#include "TitleBar.hpp"

#include <QMainWindow>
#include <QWidget>


namespace PotatoAlert::Gui {

class NativeWindow : public QWidget
{
	Q_OBJECT

private:
	const Client::ServiceProvider& m_services;

	QMainWindow* m_mainWindow;
	TitleBar* m_titleBar = new TitleBar(this);

public:
	explicit NativeWindow(QMainWindow* mainWindow, QWidget* parent = nullptr);
	static void RequestFocus();

private:
	QMainWindow* m_mainWindow;
	TitleBar* m_titleBar = new TitleBar(this);

	void Init();

	void hideEvent(QHideEvent* event) override;
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
};

}  // namespace PotatoAlert::Gui

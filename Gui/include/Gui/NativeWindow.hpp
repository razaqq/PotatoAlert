// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include "Gui/TitleBar.hpp"

#include <QMainWindow>
#include <QWidget>


namespace PotatoAlert::Gui {

class NativeWindow : public QWidget
{
	Q_OBJECT

public:
	explicit NativeWindow(const Client::ServiceProvider& serviceProvider, QMainWindow* mainWindow, QWidget* parent = nullptr);
	static void RequestFocus();

private:
	void Init();

	void hideEvent(QHideEvent* event) override;
	void showEvent(QShowEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
#else
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
#endif

private:
	const Client::ServiceProvider& m_services;

	QMainWindow* m_mainWindow;
	TitleBar* m_titleBar = new TitleBar(this);
	bool m_isInitialShow = true;
};

}  // namespace PotatoAlert::Gui

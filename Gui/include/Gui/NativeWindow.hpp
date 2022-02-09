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

public:
	explicit NativeWindow(QMainWindow* mainWindow);
	static void RequestFocus();

private:
	void Init();
	QMainWindow* m_mainWindow;

	static constexpr int m_borderWidth = 4;

	TitleBar* m_titleBar = new TitleBar(this);

	void closeEvent(QCloseEvent* event) override;
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
};

}  // namespace PotatoAlert::Gui

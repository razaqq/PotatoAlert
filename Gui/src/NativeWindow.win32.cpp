// Copyright 2022 <github.com/razaqq>

#include "Gui/NativeWindow.hpp"

#define WIN32_USER
#include "win32.h"


UINT WM_SHOWME = RegisterWindowMessageA("WM_SHOWME");

using PotatoAlert::Gui::NativeWindow;

void NativeWindow::RequestFocus()
{
	PostMessageA(HWND_BROADCAST, WM_SHOWME, 0, 0);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool NativeWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
#else
bool NativeWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
#endif
{
	const MSG* msg = static_cast<MSG*>(message);
	if (msg->message == WM_SHOWME)
	{
		show();
		setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		raise();
		activateWindow();
	}
	return QWidget::nativeEvent(eventType, message, result);
}

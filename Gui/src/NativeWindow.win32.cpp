// Copyright 2022 <github.com/razaqq>

#include "Gui/NativeWindow.hpp"

#define WIN32_USER
#define WIN32_WINMESSAGES
#define WIN32_MSG
#include "win32.h"


namespace {
	UINT WmShowMe()
	{
		static UINT wmShowMe = RegisterWindowMessageA("WM_SHOWME");
		return wmShowMe;
	}
}

using PotatoAlert::Gui::NativeWindow;

void NativeWindow::RequestFocus()
{
	PostMessageA(HWND_BROADCAST, WmShowMe(), 0, 0);
}

bool NativeWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
	const MSG* msg = static_cast<MSG*>(message);
	if (msg->message == WmShowMe())
	{
		show();
		setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
		raise();
		activateWindow();
	}
	return QWidget::nativeEvent(eventType, message, result);
}

// Copyright 2020 <github.com/razaqq>

#include "FramelessDialog.hpp"

#define WIN32_USER
#define WIN32_WINSTYLES
#define WIN32_WINOFFSETS
#define WIN32_WINMESSAGES
#define WIN32_MSG
#define WIN32_TEXTMETRIC
#define WIN32_GDI
#define WIN32_CTLMGR
#include "win32.h"

#include <dwmapi.h>

#include <QDialog>
#include <QWidget>
#include <QWindow>


using PotatoAlert::FramelessDialog;

FramelessDialog::FramelessDialog(QWidget* parent) : QDialog(parent)
{
	this->setParent(parent);

	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
	this->setWindowModality(Qt::WindowModal);
}

void FramelessDialog::showEvent(QShowEvent* event)
{
	// edit underlying native window
	const HWND winId = reinterpret_cast<HWND>(this->windowHandle()->winId());

	// edit style
	const LONG style = GetWindowLongA(winId, GWL_STYLE);
	SetWindowLongPtrA(winId, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);

	// add shadow
	const MARGINS shadow = { 1, 1, 1, 1 };
	DwmExtendFrameIntoClientArea(winId, &shadow);

	QDialog::showEvent(event);
}

bool FramelessDialog::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	MSG* msg = static_cast<MSG*>(message);
	if (msg->message == WM_NCCALCSIZE)
	{
		NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
		if (params.rgrc[0].top != 0)
			params.rgrc[0].top -= 1;

		// kill the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
		*result = WVR_REDRAW;
		return true;
	}
	return QWidget::nativeEvent(eventType, message, result);
}

// Copyright 2020 <github.com/razaqq>

#include "FramelessDialog.hpp"

#include <QDialog>
#include <QWidget>
#include <QWindow>

#include <dwmapi.h>
#include "win32.h"


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
	HWND winId = reinterpret_cast<HWND>(this->windowHandle()->winId());

	// edit style
	LONG style = GetWindowLong(winId, GWL_STYLE);
	SetWindowLongPtr(winId, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);

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

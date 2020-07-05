// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QPointF>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QWindow>
#include <QEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QPaintEvent>
#include <QApplication>
#include "NativeWindow.h"
#include "TitleBar.h"
#include "Config.h"
#include <atlstr.h>

#include <Windows.h>  // windows only
#include <dwmapi.h>
#include <gdiplus.h>
#include <Dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")


using PotatoAlert::NativeWindow;

NativeWindow::NativeWindow(QMainWindow* mainWindow, Config* c) : QWidget()
{
	this->c = c;
	this->mainWindow = mainWindow;
	this->mainWindow->setParent(this);
	this->init();
}

void NativeWindow::closeEvent(QCloseEvent* event) {
	this->c->set<int>("window_height", this->height());
	this->c->set<int>("window_width", this->width());
	this->c->set<int>("window_x", this->x());
	this->c->set<int>("window_y", this->y());
	QWidget::closeEvent(event);
}

void NativeWindow::init()
{
	auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(this->titleBar);
	layout->addWidget(this->mainWindow);

	this->setLayout(layout);

	this->resize(c->get<int>("window_width"), c->get<int>("window_height"));
	this->move(c->get<int>("window_x"), c->get<int>("window_y"));

	this->setWindowFlags(this->windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

	// we need to set this as event filter for titlebar and main window to draw resize cursor
	qApp->installEventFilter(this);
	// this->setMouseTracking(true);
}

void NativeWindow::showEvent(QShowEvent* event)
{
	// edit underlaying native window
	HWND winid = (HWND)this->windowHandle()->winId();

	// edit style
	DWORD style = ::GetWindowLong(winid, GWL_STYLE);
	SetWindowLongPtr(winid, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);

	// add shadow
	const MARGINS shadow = { 1, 1, 1, 1 };
	DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);

	QWidget::showEvent(event);
}

bool NativeWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	MSG* msg = reinterpret_cast<MSG*>(message);
	switch (msg->message)
	{
	case WM_NCCALCSIZE:
	{
		NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
		if (params.rgrc[0].top != 0)
			params.rgrc[0].top -= 1;

		// kill the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
		*result = WVR_REDRAW;
		return true;
	}
	default:
		return QWidget::nativeEvent(eventType, message, result);
	}
}

bool NativeWindow::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::MouseMove)
	{
		this->handleMouseMoveEvent((QMouseEvent*)event);
		return false;
	}
	else if (event->type() == QEvent::MouseButtonPress)
	{
		return this->handleMousePressEvent((QMouseEvent*)event);
	}
	return QWidget::eventFilter(object, event);
}

bool NativeWindow::handleMousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		Qt::Edges e = this->mouseLocation(event);
		if (e != 0x00000 && this->windowState() != Qt::WindowMaximized)
		{
			this->windowHandle()->startSystemResize(e);
			return true;
		}

		int y = event->globalY() - this->y();
		int x = event->globalX() - this->x();
		if (y > 0 && y < this->titleBar->height() && x < this->titleBar->btnStartX())
		{
			this->windowHandle()->startSystemMove();
			return true;
		}
	}
	return false;
}

void NativeWindow::handleMouseMoveEvent(QMouseEvent* event)
{
	if (this->windowState() == Qt::WindowMaximized) {
		this->unsetCursor();
		return;
	}

	switch (this->mouseLocation(event))
	{
	case Qt::TopEdge | Qt::RightEdge:
	case Qt::BottomEdge | Qt::LeftEdge:
		this->setCursor(Qt::SizeBDiagCursor);
		break;
	case Qt::TopEdge | Qt::LeftEdge:
	case Qt::BottomEdge | Qt::RightEdge:
		this->setCursor(Qt::SizeFDiagCursor);
		break;
	case Qt::TopEdge:
	case Qt::BottomEdge:
		this->setCursor(Qt::SizeVerCursor);
		break;
	case Qt::RightEdge:
	case Qt::LeftEdge:
		this->setCursor(Qt::SizeHorCursor);
		break;
	default:
		this->unsetCursor();
	}
}

Qt::Edges NativeWindow::mouseLocation(QMouseEvent* event)
{
	int x = event->globalX() - this->x();
	int y = event->globalY() - this->y();

	Qt::Edges e;
	if (this->width() - x < this->borderWidth)
		e |= Qt::RightEdge;
	if (x < this->borderWidth)
		e |= Qt::LeftEdge;
	if (this->height() - y < this->borderWidth)
		e |= Qt::BottomEdge;
	if (y < this->borderWidth)
		e |= Qt::TopEdge;
	return e;
}

void NativeWindow::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
        if (this->windowState() == Qt::WindowMaximized)
            this->setContentsMargins(7, 7, 7, 7);
        else
            this->setContentsMargins(0, 0, 0, 0);
    }
}

bool NativeWindow::confirmUpdate()
{
    // CString text;
	// text.LoadStringW(104);
	// QString::fromWCharArray((LPCTSTR)text, text.GetLength());
	return false;  // TODO
}

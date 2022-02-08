#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "FramelessHelper/FramelessHelper_win32.hpp"
#include "FramelessHelper/FramelessUtilities.hpp"

#include <QDebug>
#include <QVariant>
#include <QCoreApplication>
#include <QWindow>
#include <QtCore/qt_windows.h>
#include <shellapi.h>


#ifndef WM_NCUAHDRAWCAPTION
// Not documented, only available since Windows Vista
#define WM_NCUAHDRAWCAPTION 0x00AE
#endif

#ifndef WM_NCUAHDRAWFRAME
// Not documented, only available since Windows Vista
#define WM_NCUAHDRAWFRAME 0x00AF
#endif

#ifndef ABM_GETAUTOHIDEBAREX
// Only available since Windows 8.1
#define ABM_GETAUTOHIDEBAREX 0x0000000b
#endif

#ifndef IsMinimized
// Only available since Windows 2000
#define IsMinimized(h) IsIconic(h)
#endif

#ifndef IsMaximized
// Only available since Windows 2000
#define IsMaximized(h) IsZoomed(h)
#endif

#ifndef GET_X_LPARAM
// Only available since Windows 2000
#define GET_X_LPARAM(lp) ((int) (short) LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
// Only available since Windows 2000
#define GET_Y_LPARAM(lp) ((int) (short) HIWORD(lp))
#endif

static inline bool shouldHaveWindowFrame()
{
	if (Utilities::shouldUseNativeTitleBar())
		return true;

	const bool should = qEnvironmentVariableIsSet(_flh_global::_flh_preserveNativeFrame_flag);
	const bool force = qEnvironmentVariableIsSet(_flh_global::_flh_forcePreserveNativeFrame_flag);
	if (should || force)
	{
		if (force)
			return true;
		if (should)
			return Utilities::isWin10OrGreater();
	}
	return false;
}

// The thickness of an auto-hide taskbar in pixels.
static const int kAutoHideTaskbarThicknessPx = 2;
static const int kAutoHideTaskbarThicknessPy = kAutoHideTaskbarThicknessPx;

static QScopedPointer<FramelessHelperWin> g_instance;

static inline void setup()
{
	if (g_instance.isNull())
	{
		g_instance.reset(new FramelessHelperWin);
		qApp->installNativeEventFilter(g_instance.data());
	}
}

static inline void installHelper(QWindow *window, const bool enable)
{
	Q_ASSERT(window);
	if (!window)
		return;
	window->setProperty(_flh_global::_flh_framelessEnabled_flag, enable);
	Utilities::updateQtFrameMargins(window, enable);
	Utilities::updateFrameMargins(window, !enable);
	Utilities::triggerFrameChange(window);
}

FramelessHelperWin::FramelessHelperWin() = default;

FramelessHelperWin::~FramelessHelperWin()
{
	if (!g_instance.isNull())
		qApp->removeNativeEventFilter(g_instance.data());
}

void FramelessHelperWin::addFramelessWindow(QWindow *window)
{
	Q_ASSERT(window);
	setup();
	installHelper(window, true);
}

void FramelessHelperWin::setIgnoredObjects(QWindow *window, const QObjectList &objects)
{
	Q_ASSERT(window);
	if (!window)
		return;
	window->setProperty(_flh_global::_flh_ignoredObjects_flag, QVariant::fromValue(objects));
}

QObjectList FramelessHelperWin::getIgnoredObjects(const QWindow *window)
{
	Q_ASSERT(window);
	if (!window) {
		return {};
	}
	return qvariant_cast<QObjectList>(window->property(_flh_global::_flh_ignoredObjects_flag));
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool FramelessHelperWin::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool FramelessHelperWin::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
	if (!result)
		return false;

	if (eventType != "windows_generic_MSG")
		return false;

	const auto msg = static_cast<LPMSG>(message);

	if (!msg || !msg->hwnd)
		return false;
	const QWindow *window = Utilities::findWindow(reinterpret_cast<WId>(msg->hwnd));
	if (!window || (!window->property(_flh_global::_flh_framelessEnabled_flag).toBool()))
		return false;

	switch (msg->message)
	{
	case WM_NCCALCSIZE:
	{
		if (Utilities::shouldUseNativeTitleBar())
			break;

		if (!msg->wParam)
		{
			*result = 0;
			return true;
		}
		bool nonClientAreaExists = false;
		const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(msg->lParam)->rgrc[0]);

		if (shouldHaveWindowFrame())
		{
			// Store the original top before the default window proc
			// applies the default frame.
			const LONG originalTop = clientRect->top;
			// Apply the default frame
			const LRESULT ret = DefWindowProcW(msg->hwnd, WM_NCCALCSIZE, msg->wParam, msg->lParam);
			if (ret != 0)
			{
				*result = ret;
				return true;
			}
			// Re-apply the original top from before the size of the
			// default frame was applied.
			clientRect->top = originalTop;
		}

		if (IsMaximized(msg->hwnd) && (window->windowState() != Qt::WindowFullScreen))
		{
			const int bh = getSystemMetric(window, Utilities::SystemMetric::BorderHeight, true);
			clientRect->top += bh;
			if (!shouldHaveWindowFrame())
			{
				clientRect->bottom -= bh;
				const int bw = getSystemMetric(window, Utilities::SystemMetric::BorderWidth, true);
				clientRect->left += bw;
				clientRect->right -= bw;
			}
			nonClientAreaExists = true;
		}

		if (IsMaximized(msg->hwnd))
		{
			APPBARDATA abd;
			SecureZeroMemory(&abd, sizeof(abd));
			abd.cbSize = sizeof(abd);
			const UINT taskbarState = SHAppBarMessage(ABM_GETSTATE, &abd);

			// First, check if we have an auto-hide taskbar at all:
			if (taskbarState & ABS_AUTOHIDE) {
				bool top, bottom, left, right;
				if (Utilities::isWin8Point1OrGreater()) {
					MONITORINFO monitorInfo;
					SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
					monitorInfo.cbSize = sizeof(monitorInfo);

					HMONITOR monitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
					GetMonitorInfoW(monitor, &monitorInfo);

					const auto hasAutohideTaskbar = [&monitorInfo](const UINT edge) -> bool
					{
						APPBARDATA _abd;
						SecureZeroMemory(&_abd, sizeof(_abd));
						_abd.cbSize = sizeof(_abd);
						_abd.uEdge = edge;
						_abd.rc = monitorInfo.rcMonitor;
						const auto hTaskbar = reinterpret_cast<HWND>(SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &_abd));
						return hTaskbar != nullptr;
					};
					top = hasAutohideTaskbar(ABE_TOP);
					bottom = hasAutohideTaskbar(ABE_BOTTOM);
					left = hasAutohideTaskbar(ABE_LEFT);
					right = hasAutohideTaskbar(ABE_RIGHT);
				}
				else
				{
					int edge = -1;
					APPBARDATA _abd;
					SecureZeroMemory(&_abd, sizeof(_abd));
					_abd.cbSize = sizeof(_abd);
					_abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
					if (_abd.hWnd)
					{
						HMONITOR windowMonitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
						HMONITOR taskbarMonitor = MonitorFromWindow(_abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
						if (taskbarMonitor == windowMonitor)
						{
							SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
							edge = _abd.uEdge;
						}
					}
					top = edge == ABE_TOP;
					bottom = edge == ABE_BOTTOM;
					left = edge == ABE_LEFT;
					right = edge == ABE_RIGHT;
				}

				if (top)
				{
					// Peculiarly, when we're fullscreen,
					clientRect->top += kAutoHideTaskbarThicknessPy;
					nonClientAreaExists = true;
				}
				else if (bottom)
				{
					clientRect->bottom -= kAutoHideTaskbarThicknessPy;
					nonClientAreaExists = true;
				}
				else if (left)
				{
					clientRect->left += kAutoHideTaskbarThicknessPx;
					nonClientAreaExists = true;
				}
				else if (right)
				{
					clientRect->right -= kAutoHideTaskbarThicknessPx;
					nonClientAreaExists = true;
				}
			}
		}

		// Fix the flickering issue while resizing.
		// "clientRect->right += 1;" also works.
		// The only draw back of this small trick is it will affect
		// Qt's coordinate system. It makes the "canvas" of the window
		// larger than it should be. Be careful if you need to paint
		// something manually either through QPainter or Qt Quick.
		clientRect->bottom += 1;

		*result = nonClientAreaExists ? 0 : WVR_REDRAW;
		return true;
	}
	case WM_NCUAHDRAWCAPTION:
	case WM_NCUAHDRAWFRAME:
	{
		if (shouldHaveWindowFrame())
		{
			break;
		}
		else
		{
			*result = 0;
			return true;
		}
	}
	case WM_NCPAINT:
	{
		if (!Utilities::isDwmBlurAvailable() && !shouldHaveWindowFrame())
		{
			*result = 0;
			return true;
		}
		else
		{
			break;
		}
	}
	case WM_NCACTIVATE:
	{
		if (shouldHaveWindowFrame())
			break;

		if (Utilities::isDwmBlurAvailable())
		{
			*result = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, -1);
		}
		else
		{
			if (static_cast<BOOL>(msg->wParam))
			{
				*result = FALSE;
			}
			else
			{
				*result = TRUE;
			}
		}
		return true;
	}
	case WM_NCHITTEST:
	{
		if (Utilities::shouldUseNativeTitleBar())
			break;

		const qreal dpr = window->devicePixelRatio();
		const QPointF globalMouse = QCursor::pos(window->screen()) * dpr;
		POINT winLocalMouse = { qRound(globalMouse.x()), qRound(globalMouse.y()) };
		ScreenToClient(msg->hwnd, &winLocalMouse);
		const QPointF localMouse = { static_cast<qreal>(winLocalMouse.x), static_cast<qreal>(winLocalMouse.y) };
		const bool isInIgnoreObjects = Utilities::isMouseInSpecificObjects(globalMouse, getIgnoredObjects(window), dpr);
		const int bh = getSystemMetric(window, Utilities::SystemMetric::BorderHeight, true);
		const int tbh = getSystemMetric(window, Utilities::SystemMetric::TitleBarHeight, true);
		const bool isTitleBar = (localMouse.y() <= tbh) && !isInIgnoreObjects;
		const bool isTop = localMouse.y() <= bh;

		if (shouldHaveWindowFrame())
		{
			const LRESULT originalRet = DefWindowProcW(msg->hwnd, WM_NCHITTEST, msg->wParam, msg->lParam);
			if (originalRet != HTCLIENT)
			{
				*result = originalRet;
				return true;
			}

			if (!IsMaximized(msg->hwnd) && isTop)
			{
				*result = HTTOP;
				return true;
			}
			if (isTitleBar)
			{
				*result = HTCAPTION;
				return true;
			}
			*result = HTCLIENT;
			return true;
		}
		else
		{
			const auto getHitTestResult = [msg, isTitleBar, &localMouse, bh, isTop, window]() -> LRESULT
			{
				if (IsMaximized(msg->hwnd))
				{
					if (isTitleBar)
						return HTCAPTION;
					return HTCLIENT;
				}

				RECT clientRect = {0, 0, 0, 0};
				GetClientRect(msg->hwnd, &clientRect);
				const LONG ww = clientRect.right;
				const LONG wh = clientRect.bottom;
				const int bw = getSystemMetric(window, Utilities::SystemMetric::BorderWidth, true);

				const bool isBottom = (localMouse.y() >= (wh - bh));
				// Make the border a little wider to let the user easy to resize on corners.
				const int factor = (isTop || isBottom) ? 2 : 1;
				const bool isLeft = (localMouse.x() <= (bw * factor));
				const bool isRight = (localMouse.x() >= (ww - (bw * factor)));
				const bool fixedSize = Utilities::isWindowFixedSize(window);

				const auto getBorderValue = [fixedSize](int value) -> int { return fixedSize ? HTBORDER : value; };
				if (isTop)
				{
					if (isLeft)
						return getBorderValue(HTTOPLEFT);
					if (isRight)
						return getBorderValue(HTTOPRIGHT);
					return getBorderValue(HTTOP);
				}
				if (isBottom)
				{
					if (isLeft)
						return getBorderValue(HTBOTTOMLEFT);
					if (isRight)
						return getBorderValue(HTBOTTOMRIGHT);
					return getBorderValue(HTBOTTOM);
				}
				if (isLeft)
					return getBorderValue(HTLEFT);
				if (isRight)
					return getBorderValue(HTRIGHT);
				if (isTitleBar)
					return HTCAPTION;
				return HTCLIENT;
			};
			*result = getHitTestResult();
			return true;
		}
	}
	case WM_SETICON:
	case WM_SETTEXT:
	{
		if (Utilities::shouldUseNativeTitleBar())
			break;

		const auto oldStyle = GetWindowLongPtrW(msg->hwnd, GWL_STYLE);
		SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle & ~WS_VISIBLE);
		Utilities::triggerFrameChange(window);
		const LRESULT ret = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
		SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle);
		Utilities::triggerFrameChange(window);
		*result = ret;
		return true;
	}
	default:
		break;
	}
	return false;
}

void FramelessHelperWin::setBorderWidth(QWindow *window, const int bw)
{
	Q_ASSERT(window);
	if (!window)
		return;
	window->setProperty(_flh_global::_flh_borderWidth_flag, bw);
}

void FramelessHelperWin::setBorderHeight(QWindow *window, const int bh)
{
	Q_ASSERT(window);
	if (!window)
		return;
	window->setProperty(_flh_global::_flh_borderHeight_flag, bh);
}

void FramelessHelperWin::setTitleBarHeight(QWindow *window, const int tbh)
{
	Q_ASSERT(window);
	if (!window)
		return;
	window->setProperty(_flh_global::_flh_titleBarHeight_flag, tbh);
}

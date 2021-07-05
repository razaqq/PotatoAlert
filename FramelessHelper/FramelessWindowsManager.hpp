#pragma once

#include "FramelessHelper_global.hpp"
#include <QRect>


QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QObject)
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_END_NAMESPACE

class FramelessWindowsManager
{
	Q_DISABLE_COPY_MOVE(FramelessWindowsManager)

public:
	explicit FramelessWindowsManager();
	~FramelessWindowsManager() = default;

	static void addWindow(const QWindow *window);

	static void addIgnoreObject(const QWindow *window, QObject *object);

	[[maybe_unused]] static int getBorderWidth(const QWindow *window);
	[[maybe_unused]] static void setBorderWidth(const QWindow *window, int value);
	[[maybe_unused]] static int getBorderHeight(const QWindow *window);
	[[maybe_unused]] static void setBorderHeight(const QWindow *window, int value);
	[[maybe_unused]] static int getTitleBarHeight(const QWindow *window);
	[[maybe_unused]] static void setTitleBarHeight(const QWindow *window, int value);
	[[maybe_unused]] static bool getResizable(const QWindow *window);
	[[maybe_unused]] static void setResizable(const QWindow *window, bool value = true);
};

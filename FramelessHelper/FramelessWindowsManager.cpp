#include "FramelessWindowsManager.hpp"
#include "FramelessUtilities.hpp"
#include "FramelessHelper_win32.hpp"
#include <QWindow>
#include <QScreen>


FramelessWindowsManager::FramelessWindowsManager() = default;

void FramelessWindowsManager::addWindow(const QWindow *window)
{
	Q_ASSERT(window);
	if (!window)
		return;
	const auto win = const_cast<QWindow *>(window);
	FramelessHelperWin::addFramelessWindow(win);

	/*
	QObject::connect(win, &QWindow::screenChanged, [win](QScreen *screen)
	{
		Q_UNUSED(screen)
		win->resize(win->size());
	});
	*/
}

void FramelessWindowsManager::addIgnoreObject(const QWindow *window, QObject *object)
{
	Q_ASSERT(window);
	if (!window)
		return;
	QObjectList objects = FramelessHelperWin::getIgnoredObjects(window);
	objects.append(object);
	FramelessHelperWin::setIgnoredObjects(const_cast<QWindow *>(window), objects);
}

[[maybe_unused]] int FramelessWindowsManager::getBorderWidth(const QWindow *window)
{
	Q_ASSERT(window);
	if (!window)
		return 0;
	return Utilities::getSystemMetric(window, Utilities::SystemMetric::BorderWidth, false);
}

[[maybe_unused]] void FramelessWindowsManager::setBorderWidth(const QWindow *window, const int value)
{
	Q_ASSERT(window);
	if (!window)
		return;
	FramelessHelperWin::setBorderWidth(const_cast<QWindow *>(window), value);
}

[[maybe_unused]] int FramelessWindowsManager::getBorderHeight(const QWindow *window)
{
	Q_ASSERT(window);
	if (!window)
		return 0;
	return Utilities::getSystemMetric(window, Utilities::SystemMetric::BorderHeight, false);
}

[[maybe_unused]] void FramelessWindowsManager::setBorderHeight(const QWindow *window, const int value)
{
	Q_ASSERT(window);
	if (!window)
		return;
	FramelessHelperWin::setBorderHeight(const_cast<QWindow *>(window), value);
}

[[maybe_unused]] int FramelessWindowsManager::getTitleBarHeight(const QWindow *window)
{
	Q_ASSERT(window);
	if (!window)
		return 0;
	return Utilities::getSystemMetric(window, Utilities::SystemMetric::TitleBarHeight, false);
}

[[maybe_unused]] void FramelessWindowsManager::setTitleBarHeight(const QWindow *window, const int value)
{
	Q_ASSERT(window);
	if (!window)
		return;
	FramelessHelperWin::setTitleBarHeight(const_cast<QWindow *>(window), value);
}

[[maybe_unused]] bool FramelessWindowsManager::getResizable(const QWindow *window)
{
	Q_ASSERT(window);
	if (!window)
		return false;
	return !Utilities::isWindowFixedSize(window);
}

[[maybe_unused]] void FramelessWindowsManager::setResizable(const QWindow *window, const bool value)
{
	Q_ASSERT(window);
	if (!window)
		return;
	const_cast<QWindow *>(window)->setFlag(Qt::MSWindowsFixedSizeDialogHint, !value);
}

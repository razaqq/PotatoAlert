#pragma once

#include "FramelessHelper_global.hpp"
#include <QColor>
#include <QWindow>

namespace Utilities {

enum class SystemMetric
{
	BorderWidth,
	BorderHeight,
	TitleBarHeight
};

enum class DesktopWallpaperAspectStyle
{
	Central,
	Tiled,
	IgnoreRatioFit, // Stretch
	KeepRatioFit, // Fit
	KeepRatioByExpanding, // Fill
	Span
};

// Common
[[maybe_unused]] bool shouldUseWallpaperBlur();
bool shouldUseTraditionalBlur();
[[maybe_unused]] bool setBlurEffectEnabled(const QWindow *window, bool enabled, const QColor &gradientColor = {});

int getSystemMetric(const QWindow *window, SystemMetric metric, bool dpiAware, bool forceSystemValue = false);

[[maybe_unused]] bool isLightThemeEnabled();
bool isDarkThemeEnabled();

QWindow *findWindow(WId winId);

[[maybe_unused]] QImage getDesktopWallpaperImage(int screen = -1);
[[maybe_unused]] QColor getDesktopBackgroundColor(int screen = -1);
[[maybe_unused]] DesktopWallpaperAspectStyle getDesktopWallpaperAspectStyle(int screen = -1);

[[maybe_unused]] QRect getScreenAvailableGeometry(const QWindow *window);
[[maybe_unused]] QRect getScreenAvailableGeometry(const QPoint &pos);

[[maybe_unused]] QRect getScreenGeometry(const QWindow *window);
[[maybe_unused]] QRect getScreenGeometry(const QPoint &pos);

[[maybe_unused]] QRect alignedRect(Qt::LayoutDirection direction, Qt::Alignment alignment, const QSize &size, const QRect &rectangle);

[[maybe_unused]] void blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
[[maybe_unused]] void blurImage(QPainter *painter, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);

bool disableExtraProcessingForBlur();
bool forceEnableTraditionalBlur();
bool forceDisableWallpaperBlur();
bool shouldUseNativeTitleBar();

bool isWindowFixedSize(const QWindow *window);

bool isMouseInSpecificObjects(const QPointF &mousePos, const QObjectList &objects, qreal dpr = 1.0);

#ifdef Q_OS_WINDOWS
// Windows specific
bool isWin7OrGreater();
bool isWin8OrGreater();
bool isWin8Point1OrGreater();
bool isWin10OrGreater();
bool isWin10OrGreater(int subVer);

bool isDwmBlurAvailable();
bool isOfficialMSWin10AcrylicBlurAvailable();

bool isColorizationEnabled();
QColor getColorizationColor();

[[maybe_unused]] bool isHighContrastModeEnabled();
[[maybe_unused]] bool isDarkFrameEnabled(const QWindow *window);
[[maybe_unused]] bool isTransparencyEffectEnabled();

void triggerFrameChange(const QWindow *window);
void updateFrameMargins(const QWindow *window, bool reset);
void updateQtFrameMargins(QWindow *window, bool enable);

quint32 getWindowDpi(const QWindow *window);
[[maybe_unused]] QMargins getWindowNativeFrameMargins(const QWindow *window);
[[maybe_unused]] [[maybe_unused]] QColor getNativeWindowFrameColor(bool isActive = true);

[[maybe_unused]] void displaySystemMenu(const QWindow *window, const QPoint &pos = {});
#endif

}

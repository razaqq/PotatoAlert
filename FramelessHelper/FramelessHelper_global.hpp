#pragma once

#include <QtCore/qglobal.h>


#if defined(Q_OS_WIN) && !defined(Q_OS_WINDOWS)
#define Q_OS_WINDOWS
#endif

#ifndef Q_DISABLE_MOVE
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#endif

#ifndef Q_DISABLE_COPY_MOVE
#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
#define qAsConst(i) std::as_const(i)
#endif

namespace _flh_global {

[[maybe_unused]] const char _flh_framelessEnabled_flag[] = "_FRAMELESSHELPER_FRAMELESS_MODE_ENABLED";
[[maybe_unused]] const char _flh_borderWidth_flag[] = "_FRAMELESSHELPER_WINDOW_BORDER_WIDTH";
[[maybe_unused]] const char _flh_borderHeight_flag[] = "_FRAMELESSHELPER_WINDOW_BORDER_HEIGHT";
[[maybe_unused]] const char _flh_titleBarHeight_flag[] = "_FRAMELESSHELPER_WINDOW_TITLE_BAR_HEIGHT";
[[maybe_unused]] const char _flh_ignoredObjects_flag[] = "_FRAMELESSHELPER_WINDOW_TITLE_BAR_IGNORED_OBJECTS";
[[maybe_unused]] const char _flh_acrylic_blurEnabled_flag[] = "_FRAMELESSHELPER_BLUR_ENABLED";
[[maybe_unused]] const char _flh_acrylic_blurMode_flag[] = "_FRAMELESSHELPER_BLUR_MODE";
[[maybe_unused]] const char _flh_acrylic_gradientColor_flag[] = "_FRAMELESSHELPER_BLUR_GRADIENT_COLOR";
[[maybe_unused]] const char _flh_acrylic_disableExtraProcess[] = "_FRAMELESSHELPER_DISABLE_EXTRA_PROCESS_FOR_BLUR";
[[maybe_unused]] const char _flh_acrylic_forceEnableOfficialMSWin10AcrylicBlur_flag[] = "_FRAMELESSHELPER_FORCE_ENABLE_MSWIN10_OFFICIAL_ACRYLIC_BLUR";
[[maybe_unused]] const char _flh_acrylic_forceEnableTraditionalBlur_flag[] = "_FRAMELESSHELPER_FORCE_ENABLE_TRADITIONAL_BLUR";
[[maybe_unused]] const char _flh_acrylic_forceDisableWallpaperBlur_flag[] = "_FRAMELESSHELPER_FORCE_DISABLE_WALLPAPER_BLUR";
[[maybe_unused]] const char _flh_useNativeTitleBar_flag[] = "_FRAMELESSHELPER_USE_NATIVE_TITLE_BAR";
[[maybe_unused]] const char _flh_preserveNativeFrame_flag[] = "_FRAMELESSHELPER_PRESERVE_NATIVE_WINDOW_FRAME";
[[maybe_unused]] const char _flh_forcePreserveNativeFrame_flag[] = "_FRAMELESSHELPER_FORCE_PRESERVE_NATIVE_WINDOW_FRAME";

}

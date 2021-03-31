#pragma once

#include "FramelessHelper_global.hpp"
#include <QAbstractNativeEventFilter>
#include <QObject>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_END_NAMESPACE

class FramelessHelperWin : public QAbstractNativeEventFilter
{
    Q_DISABLE_COPY_MOVE(FramelessHelperWin)

public:
    explicit FramelessHelperWin();
    ~FramelessHelperWin() override;

    static void addFramelessWindow(QWindow *window);

    static void setIgnoredObjects(QWindow *window, const QObjectList &objects);
    static QObjectList getIgnoredObjects(const QWindow *window);

    static void setBorderWidth(QWindow *window, int bw);
    static void setBorderHeight(QWindow *window, int bh);
    static void setTitleBarHeight(QWindow *window, int tbh);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif
};

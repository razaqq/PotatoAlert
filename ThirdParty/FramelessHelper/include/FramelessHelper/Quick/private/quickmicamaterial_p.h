/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "framelesshelperquick_global.h"
#include "quickmicamaterial.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

class WallpaperImageNode;

class FRAMELESSHELPER_QUICK_API QuickMicaMaterialPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QuickMicaMaterialPrivate)
    Q_DECLARE_PUBLIC(QuickMicaMaterial)

public:
    explicit QuickMicaMaterialPrivate(QuickMicaMaterial *q);
    ~QuickMicaMaterialPrivate() override;

    Q_NODISCARD static QuickMicaMaterialPrivate *get(QuickMicaMaterial *q);
    Q_NODISCARD static const QuickMicaMaterialPrivate *get(const QuickMicaMaterial *q);

public Q_SLOTS:
    void rebindWindow();
    void forceRegenerateWallpaperImageCache();
    void appendNode(WallpaperImageNode *node);

private:
    void initialize();

private:
    QPointer<QuickMicaMaterial> q_ptr = nullptr;
    QMetaObject::Connection m_rootWindowXChangedConnection = {};
    QMetaObject::Connection m_rootWindowYChangedConnection = {};
    QList<QPointer<WallpaperImageNode>> m_nodes = {};
};

FRAMELESSHELPER_END_NAMESPACE

Q_DECLARE_METATYPE2(FRAMELESSHELPER_PREPEND_NAMESPACE(QuickMicaMaterialPrivate))

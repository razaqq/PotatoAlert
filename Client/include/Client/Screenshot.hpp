// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QRect>
#include <QList>
#include <QWidget>

#include <filesystem>


namespace PotatoAlert::Client {

bool CaptureScreenshot(QWidget* window, const std::filesystem::path& dir, const QList<QRect>& blurRects = {});

}  // namespace PotatoAlert::Client

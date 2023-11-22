// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>

#include <filesystem>


namespace PotatoAlert::Core {

bool CaptureScreenshot(QWidget* window, const std::filesystem::path& dir, const QList<QRect>& blurRects = {});

}  // namespace PotatoAlert::Core

// Copyright 2020 <github.com/razaqq>
#pragma once

#include <filesystem>
#include <QWidget>
#include <QtNetwork>


namespace fs = std::filesystem;

namespace PotatoAlert {

class Updater : QWidget
{
public:
	static bool updateAvailable();
	static void update();
	static bool download(const fs::path& targetFile);
};

}  // namespace PotatoAlert

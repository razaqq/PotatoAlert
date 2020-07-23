// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QtNetwork>


namespace PotatoAlert {

class Updater : QWidget
{
public:
	static bool updateAvailable();
	static void update();
	static void download();
};

}  // namespace PotatoAlert

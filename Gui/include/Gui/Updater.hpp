// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include "Gui/FramelessDialog.hpp"


using PotatoAlert::Gui::FramelessDialog;

namespace PotatoAlert::Gui {

class Updater : FramelessDialog
{
public:
	Updater(const Client::ServiceProvider& serviceProvider);
};

}  // namespace PotatoAlert::Gui

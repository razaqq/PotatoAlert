// Copyright 2021 <github.com/razaqq>
#pragma once

#include "FramelessDialog.hpp"
#include "Updater.hpp"


namespace PotatoUpdater {

using PotatoAlert::FramelessDialog;

class UpdaterGui : FramelessDialog
{
public:
	explicit UpdaterGui(Updater* updater);
};

}  // namespace PotatoAlert

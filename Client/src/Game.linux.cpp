// Copyright 2021 <github.com/razaqq>

#include "Client/Game.hpp"

#include <filesystem>
#include <string>
#include <vector>


namespace fs = std::filesystem;

Result<std::vector<fs::path>> PotatoAlert::Client::Game::GetDefaultGamePaths()
{
	return {};
}

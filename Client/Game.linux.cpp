// Copyright 2021 <github.com/razaqq>

#include "Game.hpp"

#include <optional>
#include <filesystem>


namespace fs = std::filesystem;

std::optional<fs::path> PotatoAlert::Client::Game::GetGamePath()
{
	return {};
}

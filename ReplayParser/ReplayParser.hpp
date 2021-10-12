// Copyright 2021 <github.com/razaqq>
#pragma once

#include "File.hpp"
#include "Packets.hpp"
#include "ReplayMeta.hpp"

#include <optional>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct ReplayFile
{
	ReplayMeta meta;
	std::vector<Packet> packets;

	static std::optional<ReplayFile> FromFile(std::string_view fileName);
	std::optional<bool> Won() const;
};

}  // namespace PotatoAlert::ReplayParser

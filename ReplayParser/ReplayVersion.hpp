// Copyright 2021 <github.com/razaqq>
#pragma once

#include <format>
#include <regex>
#include <string>
#include <vector>


namespace PotatoAlert::ReplayParser
{

struct Version
{
	uint32_t major, minor, patch, build;

	bool ParseClientExe(const std::string& str)
	{
		const std::regex del{ "," };
		const std::vector<std::string> parts(std::sregex_token_iterator(str.begin(), str.end(), del, -1), {});
		if (parts.size() != 4)
		{
			return false;
		}

		// TODO: this might be dangerous
		major = static_cast<uint32_t>(std::stoul(parts[0]));
		minor = static_cast<uint32_t>(std::stoul(parts[1]));
		patch = static_cast<uint32_t>(std::stoul(parts[2]));
		build = static_cast<uint32_t>(std::stoul(parts[3]));
		return true;
	}

	[[nodiscard]] std::string ToString() const
	{
		return std::format("{}.{}.{}", major, minor, patch);
	}
};

}

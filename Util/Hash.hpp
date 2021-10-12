// Copyright 2021 <github.com/razaqq>
#pragma once

#include <string>

namespace PotatoAlert
{

inline size_t Hash(const std::string& val)
{
	return std::hash<std::string>{}(val);
}

inline std::string HashString(const std::string& val)
{
	return std::to_string(std::hash<std::string>{}(val));
}

}  // namespace PotatoAlert

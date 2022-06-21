// Copyright 2021 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert::Core {

inline size_t Hash(std::string_view val)
{
	return std::hash<std::string_view>{}(val);
}

inline std::string HashString(std::string_view val)
{
	return std::to_string(std::hash<std::string_view>{}(val));
}

}  // namespace PotatoAlert::Core

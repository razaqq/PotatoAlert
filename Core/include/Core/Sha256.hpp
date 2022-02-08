// Copyright 2022 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert::Core {

bool Sha256(const void* data, size_t size, std::string& hash);

inline bool Sha256(std::string_view str, std::string& hash)
{
	return Sha256(str.data(), str.size(), hash);
}

template<typename RangeType, typename RangeValue>
concept range_of = std::ranges::range<RangeType> && std::is_same_v<std::ranges::range_value_t<RangeType>, RangeValue>;

template<range_of<unsigned char> R>
bool Sha256(const R& data, std::string& hash)
{
	return Sha256(data.data(), data.size(), hash);
}

template<range_of<std::byte> R>
bool Sha256(const R& data, std::string& hash)
{
	return Sha256(data.data(), data.size(), hash);
}

}  // namespace PotatoAlert::Core

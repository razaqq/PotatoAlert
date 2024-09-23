// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <string>


namespace PotatoAlert::Core {

bool Sha256(const void* data, size_t size, std::string& hash);

inline bool Sha256(std::string_view str, std::string& hash)
{
	return Sha256(str.data(), str.size(), hash);
}

inline bool Sha256(const char* str, std::string& hash)
{
	return Sha256(str, strlen(str), hash);
}

template<is_byteRange R>
bool Sha256(const R& data, std::string& hash)
{
	return Sha256(data.data(), data.size(), hash);
}

}  // namespace PotatoAlert::Core

// Copyright 2021 <github.com/razaqq>
#pragma once

#include <charconv>
#include <string>
#include <vector>


namespace PotatoAlert::String {

std::string Trim(std::string_view str);
std::string ToUpper(std::string_view str);
std::string ToLower(std::string_view str);
std::vector<std::string> Split(std::string_view str, std::string_view del);
std::string ReplaceAll(std::string_view str, std::string_view before, std::string_view after);
std::string Join(const std::vector<std::string>& v, std::string_view del);
void ReplaceAll(std::string& str, std::string_view before, std::string_view after);
bool Contains(std::string_view str, std::string_view part);
bool StartsWith(std::string_view str, std::string_view start);
bool EndsWith(std::string_view str, std::string_view end);

template<typename T>
bool ParseNumber(std::string_view str, T& value) requires std::is_integral_v<T> || std::is_floating_point_v<T>
{
	// static_assert(std::is<T> && !std::is_same_v<T, bool>, "Type must be numeric");
	auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
	return ec == std::errc();
}

bool ParseBool(std::string_view str, bool& value);

}  // namespace PotatoAlert::String

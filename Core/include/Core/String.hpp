// Copyright 2021 <github.com/razaqq>
#pragma once

#include <charconv>
#include <span>
#include <string>
#include <vector>


namespace PotatoAlert::Core::String {

std::string Trim(std::string_view str);
std::string ToUpper(std::string_view str);
std::string ToLower(std::string_view str);
std::vector<std::string> Split(std::string_view str, std::string_view del);
std::string ReplaceAll(std::string_view str, std::string_view before, std::string_view after);
std::string Join(std::span<std::string_view> v, std::string_view del);
void ReplaceAll(std::string& str, std::string_view before, std::string_view after);
bool Contains(std::string_view str, std::string_view part);
bool StartsWith(std::string_view str, std::string_view start);
bool EndsWith(std::string_view str, std::string_view end);

template<typename T>
// TODO: currently only microsoft stdlib implements from_chars for floats, enable this again later
bool ParseNumber(std::string_view str, T& value) requires std::is_integral_v<T>//  || std::is_floating_point_v<T>
{
	// static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "Type must be numeric");
	auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
	return ec == std::errc();
}

bool ParseBool(std::string_view str, bool& value);

}  // namespace PotatoAlert::Core::String

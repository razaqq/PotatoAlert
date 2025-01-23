// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Concepts.hpp"
#include "Core/Encoding.hpp"
#include "Core/Format.hpp"
#include "Core/Result.hpp"

#include <charconv>
#include <span>
#include <string>
#include <vector>


namespace PotatoAlert::Core {

#undef STR
#ifdef WIN32
	#define STR(String) L##String
#else
	#define STR(String) String
#endif

namespace String {

template<character Char>
void TrimExtraNulls(std::basic_string<Char>& str)
{
	if (size_t i = str.find_last_not_of((Char)0); i != std::string::npos)
	{
		str.resize(i+1);
	}
}

std::string Trim(std::string_view str);
std::string ToUpper(std::string_view str);
std::string ToLower(std::string_view str);
std::vector<std::string> Split(std::string_view str, std::string_view del);
std::string ReplaceAll(std::string_view str, std::string_view before, std::string_view after);
std::string Join(std::span<std::string_view> v, std::string_view del);
std::string Join(std::span<const std::string> v, std::string_view del);
void ReplaceAll(std::string& str, std::string_view before, std::string_view after);
bool Contains(std::string_view str, std::string_view part);

template<character Char>
bool StartsWith(std::basic_string_view<Char> str, std::basic_string_view<Char> start)
{
	if (start.size() > str.size()) return false;
	return str.substr(0, start.size()) == start;
}

bool EndsWith(std::string_view str, std::string_view end);

template<typename T>
bool ParseNumber(std::string_view str, T& value) requires std::is_integral_v<T> || std::is_floating_point_v<T>
{
	// static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "Type must be numeric");
	return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{};
}

bool ParseBool(std::string_view str, bool& value);

}  // namespace String

}  // namespace PotatoAlert::Core::String

struct StringWrap
{
	std::string Str;

	explicit StringWrap(const std::string& str) : Str(str) {}
	explicit StringWrap(std::string&& str) noexcept : Str(std::move(str)) {}
};

template<>
struct fmt::formatter<StringWrap, wchar_t>
{
	[[maybe_unused]] static constexpr auto parse(const basic_format_parse_context<wchar_t>& ctx)
		-> basic_format_parse_context<wchar_t>::iterator
	{
		return ctx.begin();
	}

	auto format(const StringWrap& val, buffered_context<wchar_t>& ctx) const -> buffered_context<wchar_t>::iterator
	{
		if (const PotatoAlert::Core::Result<size_t> length = PotatoAlert::Core::Utf8ToWide(val.Str))
		{
			std::wstring wStr(length.value(), '\0');
			if (PotatoAlert::Core::Utf8ToWide(val.Str, wStr))
			{
				return format_to(ctx.out(), L"{}", wStr);
			}
		}
		return format_to(ctx.out(), L"<STRING-ERROR>");
	}
};

template<>
struct fmt::formatter<StringWrap>
{
	[[maybe_unused]] static constexpr auto parse(const format_parse_context& ctx)
			-> format_parse_context::iterator
	{
		return ctx.begin();
	}

	auto format(const StringWrap& val, format_context& ctx) const -> format_context::iterator
	{
		return fmt::format_to(ctx.out(), "{}", val.Str);
	}
};

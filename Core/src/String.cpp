// Copyright 2021 <github.com/razaqq>

#include "Core/String.hpp"

#include <algorithm>
#include <charconv>
#include <locale>
#include <numeric>
#include <string>


namespace s = PotatoAlert::Core::String;

static std::string_view g_whitespaces = " \n\r\t\f\v";

static std::string ltrim(std::string_view str)
{
	if (size_t i = str.find_first_not_of(g_whitespaces); i != std::string::npos)
	{
		return std::string(str.substr(i));
	}
	return "";
}

static std::string rtrim(std::string_view str)
{
	if (size_t i = str.find_last_not_of(g_whitespaces); i != std::string::npos)
	{
		return std::string(str.substr(0, i + 1));
	}
	return "";
}

std::string s::Trim(std::string_view str)
{
	return rtrim(ltrim(str));
}

typedef std::string::value_type char_t;

static char_t upChar(char_t c)
{
	return std::use_facet<std::ctype<char_t>>(std::locale()).toupper(c);
}

static char_t lowerChar(char_t c)
{
	return std::use_facet<std::ctype<char_t>>(std::locale()).tolower(c);
}

std::string s::ToUpper(std::string_view str)
{
	std::string result;
	result.resize(str.size());
	std::transform(str.begin(), str.end(), result.begin(), upChar);
	return result;
}

std::string s::ToLower(std::string_view str)
{
	std::string result;
	result.resize(str.size());
	std::transform(str.begin(), str.end(), result.begin(), lowerChar);
	return result;
}

bool s::ParseBool(std::string_view str, bool& value)
{
	const std::string in = ToLower(Trim(str));
	if (in == "true" || in == "1")
	{
		value = true;
		return true;
	}
	if (in == "false" || in == "0")
	{
		value = false;
		return true;
	}
	return false;
}

std::vector<std::string> s::Split(std::string_view str, std::string_view del)
{
	std::vector<std::string> result;
	if (del.empty())
	{
		result.emplace_back(str);
		return result;
	}

	size_t i = 0, j = 0;
	while ((j = str.find(del, i)) != std::string_view::npos)
	{
		result.emplace_back(str.substr(i, j - i));
		i = j + 1;
	}
	result.emplace_back(str.substr(i));
	return result;
}

std::string s::Join(std::span<std::string_view> v, std::string_view del)
{
	if (v.empty()) return "";
	if (v.size() == 1) return std::string(v[0]);

	const size_t size = std::accumulate(v.begin(), v.end(), size_t(0), [](size_t current, std::string_view str) -> size_t
	{
		return current + str.size();
	});

	std::string out;
	out.reserve(size + ((v.size() - 1) * del.size()));

	for (auto begin = v.begin(); begin != v.end();)
	{
		out += *begin;

		++begin;
		if (begin != v.end())
			out += del;
	}
	return out;
}

std::string s::Join(std::span<const std::string> v, std::string_view del)
{
	if (v.empty()) return "";
	if (v.size() == 1) return v[0];

	const size_t size = std::accumulate(v.begin(), v.end(), size_t(0), [](size_t current, std::string_view str) -> size_t
	{
		return current + str.size();
	});

	std::string out;
	out.reserve(size + ((v.size() - 1) * del.size()));

	for (auto begin = v.begin(); begin != v.end();)
	{
		out += *begin;

		++begin;
		if (begin != v.end())
			out += del;
	}
	return out;
}

bool s::Contains(std::string_view str, std::string_view part)
{
	return str.find(part) != std::string_view::npos;
}

bool s::StartsWith(std::string_view str, std::string_view start)
{
	if (start.size() > str.size()) return false;
	return str.substr(0, start.size()) == start;
}

bool s::EndsWith(std::string_view str, std::string_view end)
{
	if (end.size() > str.size()) return false;
	return str.substr(str.size() - end.size(), end.size()) == end;
}

std::string s::ReplaceAll(std::string_view str, std::string_view before, std::string_view after)
{
	std::string newString;
	newString.reserve(str.length());

	size_t i = 0, j = 0;

	while ((i = str.find(before, j)) != std::string_view::npos)
	{
		newString.append(str, j, i - j);
		newString.append(after);
		j = i + before.length();
	}

	newString.append(str.substr(j));

	return newString;
}

void s::ReplaceAll(std::string& str, std::string_view before, std::string_view after)
{
	std::string newString;
	newString.reserve(str.length());

	size_t i = 0, j = 0;

	while ((i = str.find(before, j)) != std::string_view::npos)
	{
		newString.append(str, j, i - j);
		newString.append(after);
		j = i + before.length();
	}

	newString.append(str.substr(j));

	str.swap(newString);
}

// Copyright 2021 <github.com/razaqq>

#include "String.hpp"

#include <algorithm>
#include <charconv>
#include <locale>
#include <string>


namespace s = PotatoAlert::String;

static std::string_view g_whitespaces = " \n\r\t\f\v";

std::string ltrim(std::string_view str)
{
	if (size_t i = str.find_first_not_of(g_whitespaces); i != std::string::npos)
	{
		return std::string(str.substr(i));
	}
	return "";
}

std::string rtrim(std::string_view str)
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

char_t upChar(char_t c)
{
	return std::use_facet<std::ctype<char_t>>(std::locale()).toupper(c);
}

char_t lowerChar(char_t c)
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
	bool i;
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

bool s::Contains(std::string_view str, std::string_view del)
{
	return str.find(del) != std::string_view::npos;
}

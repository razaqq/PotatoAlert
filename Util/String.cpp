// Copyright 2021 <github.com/razaqq>

#include "String.hpp"

#include <algorithm>
#include <locale>
#include <string>


static std::string_view g_whitespaces = " \n\r\t\f\v";

std::string ltrim(const std::string& str)
{
	if (size_t i = str.find_first_not_of(g_whitespaces); i != std::string::npos)
	{
		return str.substr(i);
	}
	return "";
}

std::string rtrim(const std::string& str)
{
	if (size_t i = str.find_last_not_of(g_whitespaces); i != std::string::npos)
	{
		return str.substr(0, i + 1);
	}
	return "";
}

std::string PotatoAlert::Trim(const std::string& str)
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

std::string PotatoAlert::ToUpper(const std::string& str)
{
	std::string result;
	result.resize(str.size());
	std::transform(str.begin(), str.end(), result.begin(), upChar);
	return result;
}

std::string PotatoAlert::ToLower(const std::string& str)
{
	std::string result;
	result.resize(str.size());
	std::transform(str.begin(), str.end(), result.begin(), lowerChar);
	return result;
}

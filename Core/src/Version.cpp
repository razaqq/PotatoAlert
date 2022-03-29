// Copyright 2020 <github.com/razaqq>

#include "Core/String.hpp"
#include "Core/Version.hpp"

#include <format>
#include <regex>
#include <string>


using PotatoAlert::Core::Version;

Version::Version(std::string_view versionString) : m_success(false), m_version(0)
{
	Parse(versionString);
}

void Version::Parse(std::string_view str)
{
	std::match_results<std::string_view::const_iterator> matches;

#define PARSE(matches)                                     \
	for (size_t i = 1; i < (matches).size(); i++)          \
	{                                                      \
		uint8_t value;                                     \
		if (!String::ParseNumber((matches).str(i), value)) \
		{                                                  \
			return;                                        \
		}                                                  \
		m_version |= value << (0x18 - (i - 1) * 0x08);     \
	}                                                      \
	m_success = true

	const std::regex re4(R"regex((\d+)[., ]+(\d+)[., ]+(\d+)[., ]+(\d+))regex");
	if (std::regex_search(str.begin(), str.end(), matches, re4) && matches.size() == 5)
	{
		PARSE(matches);
	}

	const std::regex re3(R"regex((\d+)[., ]+(\d+)[., ]+(\d+))regex");
	if (std::regex_search(str.begin(), str.end(), matches, re3) && matches.size() == 4)
	{
		PARSE(matches);
	}

	const std::regex re2(R"regex((\d+)[., ]+(\d+))regex");
	if (std::regex_search(str.begin(), str.end(), matches, re2) && matches.size() == 3)
	{
		PARSE(matches);
	}

	const std::regex re1(R"regex((\d+))regex");
	if (std::regex_search(str.begin(), str.end(), matches, re3) && matches.size() == 2)
	{
		PARSE(matches);
	}
}

bool PotatoAlert::Core::operator==(const Version& v1, const Version& v2)
{
	if (v1.m_success != v2.m_success)
		return false;

	return v1.m_version == v2.m_version;
}

bool PotatoAlert::Core::operator!=(const Version& v1, const Version& v2)
{
	return !(v1 == v2);
}

bool PotatoAlert::Core::operator>(const Version& v1, const Version& v2)
{
	if (!v1.m_success)
		return false;
	if (!v2.m_success)
		return true;

	return v1.m_version > v2.m_version;
}

bool PotatoAlert::Core::operator<(const Version& v1, const Version& v2)
{
	if (!v2.m_success)
		return false;
	if (!v1.m_success)
		return true;

	return v1.m_version < v2.m_version;
}

std::string Version::ToString(std::string_view del, bool includeBuild) const
{
	if (includeBuild)
	{
		return std::format("{1}{0}{2}{0}{3}{0}{4}", del, Major(), Minor(), Patch(), Build());
	}
	return std::format("{1}{0}{2}{0}{3}", del, Major(), Minor(), Patch()); 
}
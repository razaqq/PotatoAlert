// Copyright 2020 <github.com/razaqq>

#include "Version.hpp"

#include "String.hpp"

#include <algorithm>
#include <format>
#include <sstream>
#include <string_view>
#include <unordered_map>


using PotatoAlert::Core::Version;

Version::Version(std::string_view versionString)
{
	Parse(versionString);
}

Version::Version(const char* versionString)
{
	Parse(versionString);
}

void Version::Parse(std::string_view versionString)
{
	std::string_view del = "";
	if (String::Contains(versionString, "."))
	{
		del = ".";
	}
	else if (String::Contains(versionString, ","))
	{
		del = ",";
	}
	else if (String::Contains(versionString, " "))
	{
		del = " ";
	}
	std::vector<std::string> split = String::Split(versionString, del);

	if (split.size() > m_data.size())
	{
		m_success = false;
		return;
	}

	for (size_t i = 0; i < split.size(); i++)
	{
		uint32_t v;
		if (String::ParseNumber<uint32_t>(String::Trim(split[i]), v))
		{
			m_data[i] = v;
		}
		else
		{
			m_success = false;
			return;
		}
	}
}

bool PotatoAlert::Core::operator==(const Version& v1, const Version& v2)
{
	if (v1.m_success != v2.m_success)
		return false;

	return v1.m_data == v2.m_data;
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

	for (size_t i = 0; i < v1.m_data.size(); i++)
	{
		if (v1.m_data[i] != v2.m_data[i])
			return v1.m_data[i] > v2.m_data[i];
	}
	return false;
}

bool PotatoAlert::Core::operator<(const Version& v1, const Version& v2)
{
	if (!v2.m_success)
		return false;
	if (!v1.m_success)
		return true;

	for (size_t i = 0; i < v1.m_data.size(); i++)
	{
		if (v1.m_data[i] != v2.m_data[i])
			return v1.m_data[i] < v2.m_data[i];
	}
	return false;
}

std::string Version::ToString(std::string_view del, bool includeBuild) const
{
	if (includeBuild)
	{
		return std::format("{1}{0}{2}{0}{3}{0}{4}", del, m_data[0], m_data[1], m_data[2], m_data[3]);
	}
	return std::format("{1}{0}{2}{0}{3}", del, m_data[0], m_data[1], m_data[2]);
}

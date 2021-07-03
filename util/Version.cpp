// Copyright 2020 <github.com/razaqq>

#include "Version.hpp"
#include <algorithm>
#include <string>
#include <sstream>


using PotatoAlert::Version;

Version::Version(const std::string& versionString)
{
	this->Parse(versionString);
}

Version::Version(const char* versionString)
{
	std::string str(versionString);
	this->Parse(str);
}

void Version::Parse(const std::string &versionString)
{
	std::istringstream ss1(versionString);
	std::string token;
	while (std::getline(ss1, token, '.'))
	{
		std::istringstream ss2(token);
		int val;
		ss2 >> val;
		if (ss2.fail())
		{
			this->m_success = false;
			return;
		}
		this->m_versionInfo.push_back(val);
	}
}

bool PotatoAlert::operator==(const Version& v1, const Version& v2)
{
	if (v1.m_success != v2.m_success)
		return false;

	const size_t j = std::max(v1.m_versionInfo.size(), v2.m_versionInfo.size());
	for (size_t i = 0; i < j; i++)
	{
		int n = i < v1.m_versionInfo.size() ? v1.m_versionInfo[i] : 0;
		int m = i < v2.m_versionInfo.size() ? v2.m_versionInfo[i] : 0;
		if (n != m)
			return false;
	}
	return true;
}

bool PotatoAlert::operator!=(const Version& v1, const Version& v2)
{
	if (v1 == v2)
		return false;
	else
		return true;
}

bool PotatoAlert::operator>(const Version& v1, const Version& v2)
{
	if (!v1.m_success)
		return false;
	if (!v2.m_success)
		return true;

	const size_t j = std::max(v1.m_versionInfo.size(), v2.m_versionInfo.size());
	for (size_t i = 0; i < j; i++)
	{
		int n = i < v1.m_versionInfo.size() ? v1.m_versionInfo[i] : 0;
		int m = i < v2.m_versionInfo.size() ? v2.m_versionInfo[i] : 0;
		if (n != m)
			return n > m;
	}
	return false;
}

bool PotatoAlert::operator<(const Version& v1, const Version& v2)
{
	if (!v2.m_success)
		return false;
	if (!v1.m_success)
		return true;

	const size_t j = std::max(v1.m_versionInfo.size(), v2.m_versionInfo.size());
	for (size_t i = 0; i < j; i++)
	{
		int n = i < v1.m_versionInfo.size() ? v1.m_versionInfo[i] : 0;
		int m = i < v2.m_versionInfo.size() ? v2.m_versionInfo[i] : 0;
		if (n != m)
			return n < m;
	}
	return false;
}

// Copyright 2020 <github.com/razaqq>

#include "Core/Format.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"

#include <ctre.hpp>

#include <string>


using PotatoAlert::Core::Version;

Version::Version(std::string_view versionString) : m_success(false), m_version(0)
{
	Parse(versionString);
}

void Version::Parse(std::string_view str)
{
	auto parse = [this](std::string_view number, uint32_t offset) -> bool
	{
		uint8_t value;
		if (!String::ParseNumber(number, value))
		{
			return false;
		}
		m_version |= value << offset;
		return true;
	};

	if (auto [whole, major, minor, patch, build] = ctre::search<R"((\d+)[., ]+(\d+)[., ]+(\d+)[., ]+(\d+))">(str); whole)
	{
		if (!parse(major.to_view(), 0x18)) return;
		if (!parse(minor.to_view(), 0x10)) return;
		if (!parse(patch.to_view(), 0x08)) return;
		if (!parse(build.to_view(), 0x00)) return;
		m_success = true;
	}

	if (auto [whole, major, minor, patch] = ctre::search<R"((\d+)[., ]+(\d+)[., ]+(\d+))">(str); whole)
	{
		if (!parse(major.to_view(), 0x18)) return;
		if (!parse(minor.to_view(), 0x10)) return;
		if (!parse(patch.to_view(), 0x08)) return;
		m_success = true;
	}


	if (auto [whole, major, minor] = ctre::search<R"((\d+)[., ]+(\d+))">(str); whole)
	{
		if (!parse(major.to_view(), 0x18)) return;
		if (!parse(minor.to_view(), 0x10)) return;
		m_success = true;
	}

	if (auto [whole, major] = ctre::search<R"((\d+))">(str); whole)
	{
		if (!parse(major.to_view(), 0x18)) return;
		m_success = true;
	}
}

std::string Version::ToString(std::string_view del, bool includeBuild) const
{
	if (includeBuild)
	{
		return fmt::format("{1}{0}{2}{0}{3}{0}{4}", del, Major(), Minor(), Patch(), Build());
	}
	return fmt::format("{1}{0}{2}{0}{3}", del, Major(), Minor(), Patch());
}

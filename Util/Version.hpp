// Copyright 2020 <github.com/razaqq>
#pragma once

#include <array>
#include <string>


namespace PotatoAlert {

struct Version
{
	Version(uint32_t major = 0, uint32_t minor = 0, uint32_t patch = 0, uint32_t build = 0)
		: m_data({major, minor, patch, build}) {}
	explicit Version(std::string_view versionString);
	explicit Version(const char* versionString);

	explicit operator bool() const { return this->m_success; }
	friend bool operator==(const Version& v1, const Version& v2);
	friend bool operator!=(const Version& v1, const Version& v2);
	friend bool operator>(const Version& v1, const Version& v2);
	friend bool operator<(const Version& v1, const Version& v2);

	[[nodiscard]] std::string ToString(std::string_view del = ".", bool includeBuild = true) const;

private:
	void Parse(std::string_view versionString);
	std::array<uint32_t, 4> m_data = {0};
	// uint32_t m_major = 0, m_minor = 0, m_patch = 0, m_build = 0;
	bool m_success = true;
};

}  // namespace PotatoAlert

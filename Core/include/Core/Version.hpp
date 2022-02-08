// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert::Core {

class Version
{
public:
	explicit Version(uint8_t major = 0, uint8_t minor = 0, uint8_t patch = 0, uint8_t build = 0)
		: m_success(true), m_version(major << 0x18 | minor << 0x10 | patch << 0x08 | build) {}
	explicit Version(std::string_view versionString);

	explicit operator bool() const { return this->m_success; }
	friend bool operator==(const Version& v1, const Version& v2);
	friend bool operator!=(const Version& v1, const Version& v2);
	friend bool operator>(const Version& v1, const Version& v2);
	friend bool operator<(const Version& v1, const Version& v2);

	[[nodiscard]] uint8_t Major() const
	{
		return m_version >> 0x18 & 0xFFU;
	}
	[[nodiscard]] uint8_t Minor() const
	{
		return m_version >> 0x10 & 0xFFU;
	}
	[[nodiscard]] uint8_t Patch() const
	{
		return m_version >> 0x08 & 0xFFU;
	}
	[[nodiscard]] uint8_t Build() const
	{
		return m_version >> 0x00 & 0xFFU;
	}

	[[nodiscard]] std::string ToString(std::string_view del = ".", bool includeBuild = true) const;

private:
	void Parse(std::string_view str);
	bool m_success;
	uint32_t m_version;
};

}  // namespace PotatoAlert::Core

// Copyright 2020 <github.com/razaqq>
#pragma once

#include <string>


namespace PotatoAlert::Core {

class Version
{
public:
	explicit Version(uint8_t major = 0u, uint8_t minor = 0u, uint8_t patch = 0u, uint8_t build = 0u)
		: m_success(true), m_version(
			static_cast<uint32_t>(major << 0x18) | static_cast<uint32_t>(minor << 0x10) |
			static_cast<uint32_t>(patch << 0x08) | static_cast<uint32_t>(build)
		) {}
	explicit Version(std::string_view versionString);

	explicit operator bool() const { return m_success; }
	bool operator==(const Version& other) const;
	bool operator!=(const Version& other) const;
	bool operator>(const Version& other) const;
	bool operator<(const Version& other) const;
	bool operator>=(const Version& other) const;
	bool operator<=(const Version& other) const;

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

	[[nodiscard]] uint32_t GetRaw() const { return m_version; }

	[[nodiscard]] std::string ToString(std::string_view del = ".", bool includeBuild = true) const;

private:
	void Parse(std::string_view str);
	bool m_success;
	uint32_t m_version;
};

}  // namespace PotatoAlert::Core

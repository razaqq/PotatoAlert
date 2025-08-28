// Copyright 2020 <github.com/razaqq>
#pragma once

#include <cstdint>
#include <string>


namespace PotatoAlert::Core {

class Version
{
public:
	explicit constexpr Version(uint8_t major = 0u, uint8_t minor = 0u, uint8_t patch = 0u, uint8_t build = 0u)
		: m_success(true), m_version(
			static_cast<uint32_t>(major << 0x18) | static_cast<uint32_t>(minor << 0x10) |
			static_cast<uint32_t>(patch << 0x08) | static_cast<uint32_t>(build)
		) {}
	explicit Version(std::string_view versionString);

	explicit operator bool() const { return m_success; }

	constexpr bool operator==(Version other) const
	{
		if (m_success != other.m_success)
			return false;

		return m_version == other.m_version;
	}

	constexpr bool operator!=(Version other) const
	{
		return !(*this == other);
	}

	constexpr bool operator>(Version other) const
	{
		if (!m_success)
			return false;
		if (!other.m_success)
			return true;

		return m_version > other.m_version;
	}

	constexpr bool operator<(Version other) const
	{
		if (!other.m_success)
			return false;
		if (!m_success)
			return true;

		return m_version < other.m_version;
	}

	constexpr bool operator<=(Version other) const
	{
		return !(*this > other);
	}

	constexpr bool operator>=(Version other) const
	{
		return !(*this < other);
	}

	[[nodiscard]] constexpr uint8_t Major() const
	{
		return m_version >> 0x18 & 0xFFU;
	}
	[[nodiscard]] constexpr uint8_t Minor() const
	{
		return m_version >> 0x10 & 0xFFU;
	}
	[[nodiscard]] constexpr uint8_t Patch() const
	{
		return m_version >> 0x08 & 0xFFU;
	}
	[[nodiscard]] constexpr uint8_t Build() const
	{
		return m_version >> 0x00 & 0xFFU;
	}

	[[nodiscard]] constexpr uint32_t GetRaw() const { return m_version; }

	constexpr operator uint32_t() const
	{
		return m_version;
	}

	[[nodiscard]] std::string ToString(std::string_view del = ".", bool includeBuild = true) const;

private:
	void Parse(std::string_view str);
	bool m_success;
	uint32_t m_version;
};

}  // namespace PotatoAlert::Core

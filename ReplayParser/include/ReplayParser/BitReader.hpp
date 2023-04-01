// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <span>


namespace PotatoAlert::ReplayParser {

class BitReader
{
public:
	explicit BitReader(std::span<const Core::Byte>& data);
	int Get(size_t nBits);
	static int BitsRequired(int numValues);

	[[nodiscard]] size_t Remaining() const
	{
		return m_bitsLeft + m_data.size() * 8;
	}

	[[nodiscard]] std::span<const Core::Byte> GetAll() const
	{
		return Core::Take(m_data, m_data.size());
	}

private:
	std::span<const Core::Byte>& m_data;
	Core::Byte m_byte;
	size_t m_bitsLeft;
};

}  // namespace PotatoAlert::ReplayParser

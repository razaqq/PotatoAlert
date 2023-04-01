// Copyright 2022 <github.com/razaqq>

#include "Core/Bytes.hpp"

#include "ReplayParser/BitReader.hpp"

#include <bit>
#include <span>


using PotatoAlert::ReplayParser::BitReader;

BitReader::BitReader(std::span<const Core::Byte>& data) : m_data(data), m_byte(0), m_bitsLeft(0)
{
}

int BitReader::Get(size_t nBits)
{
	int ret = 0;

	size_t gBits = 0;
	while (gBits < nBits)
	{
		if (m_bitsLeft == 0)
		{
			Core::TakeInto(m_data, m_byte);
			m_bitsLeft = 8;
		}

		const size_t bitsTake = std::min(nBits - gBits, m_bitsLeft);
		ret = (ret << bitsTake) | (m_byte >> (8 - bitsTake));
		m_byte <<= bitsTake;
		m_bitsLeft -= bitsTake;
		gBits += bitsTake;
	}

	return ret;
}


int BitReader::BitsRequired(uint64_t numValues)
{
	if (numValues <= 1)
	{
		return 0;
	}
	
	numValues--;

	return std::numeric_limits<decltype(numValues)>::digits - std::countl_zero(numValues);
}

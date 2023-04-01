// Copyright 2022 <github.com/razaqq>

#include "Core/Bytes.hpp"

#include "ReplayParser/BitReader.hpp"


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


int BitReader::BitsRequired(int numValues)
{
	if (numValues <= 1)
	{
		return 0;
	}
	
	numValues--;

	int nBits;
	asm(
		"bsr %1, %%eax"
		: "=a"(nBits)
		: "r"(numValues)
	);

#if 0
#ifdef _WIN32
	_asm bsr eax, numValues
	_asm mov nBits, eax
#else
	__asm__(
			"bsr %1, %%eax"
			: "=a"(nBits)
			: "r"(numValues)
	);
#endif
#endif

	return nBits + 1;
}

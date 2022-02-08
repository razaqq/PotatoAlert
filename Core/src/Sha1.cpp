// Copyright 2022 <github.com/razaqq>

#include "Core/Sha1.hpp"

#include <span>
#include <string>


using PotatoAlert::Core::Sha1;

// value, steps
constexpr unsigned int LeftRotate(unsigned int x, std::size_t n)
{
	return (x << n) ^ (x >> (32 - n));
}

void Sha1::Reset()
{
	m_h[0] = 0x67452301;
	m_h[1] = 0xEFCDAB89;
	m_h[2] = 0x98BADCFE;
	m_h[3] = 0x10325476;
	m_h[4] = 0xC3D2E1F0;

	m_blockByteIndex = 0;
	m_bitCountLow = 0;
	m_bitCountHigh = 0;
}

Sha1& Sha1::Process(const std::string& s)
{
	for (char const& byte : s)
	{
		ProcessByte(static_cast<std::byte>(byte));
	}
	return *this;
}

Sha1::operator std::string()
{
	return GetHash();
}

std::string Sha1::GetHash()
{
	unsigned int digest[5];
	GetDigest(digest);

	std::ostringstream buf;
	for (unsigned int x : digest)
	{
		buf << std::hex << std::setfill('0') << std::setw(8) << x;
	}
	return buf.str();
}

inline bool Sha1::ProcessByte(const std::byte byte)
{
	ProcessByteImpl(byte);

	if (m_bitCountLow < 0xFFFFFFF8)
	{
		m_bitCountLow += 8;
	}
	else
	{
		m_bitCountLow = 0;

		if (m_bitCountHigh <= 0xFFFFFFFE)
		{
			++m_bitCountHigh;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool Sha1::ProcessBytes(std::span<std::byte> bytes)
{
	return ProcessBlock(bytes);
}

inline void Sha1::GetDigest(DigestType& digest)
{
	// append the bit '1' to the message
	ProcessByteImpl(std::byte{ 0x80 });

	// append k bits '0', where k is the minimum number >= 0
	// such that the resulting message length is congruent to 56 (mod 64)
	// check if there is enough space for padding and bit_count
	if (m_blockByteIndex > 56)
	{
		// finish this block
		while (m_blockByteIndex != 0)
		{
			ProcessByteImpl(std::byte{ 0 });
		}

		// one more block
		while (m_blockByteIndex < 56)
		{
			ProcessByteImpl(std::byte{ 0 });
		}
	}
	else
	{
		while (m_blockByteIndex < 56)
		{
			ProcessByteImpl(std::byte{ 0 });
		}
	}

	// append length of message (before pre-processing)
	// as a 64-bit big-endian integer
	ProcessByteImpl(static_cast<std::byte>((m_bitCountHigh >> 24) & 0xFF));
	ProcessByteImpl(static_cast<std::byte>((m_bitCountHigh >> 16) & 0xFF));
	ProcessByteImpl(static_cast<std::byte>((m_bitCountHigh >> 8) & 0xFF));
	ProcessByteImpl(static_cast<std::byte>((m_bitCountHigh)&0xFF));
	ProcessByteImpl(static_cast<std::byte>((m_bitCountLow >> 24) & 0xFF));
	ProcessByteImpl(static_cast<std::byte>((m_bitCountLow >> 16) & 0xFF));
	ProcessByteImpl(static_cast<std::byte>((m_bitCountLow >> 8) & 0xFF));
	ProcessByteImpl(static_cast<std::byte>((m_bitCountLow)&0xFF));

	// get final digest
	digest[0] = m_h[0];
	digest[1] = m_h[1];
	digest[2] = m_h[2];
	digest[3] = m_h[3];
	digest[4] = m_h[4];
}

inline void Sha1::ProcessBlock()
{
	unsigned int w[80];

	for (size_t i = 0; i < 16; ++i)
	{
		w[i] = static_cast<unsigned int>(static_cast<unsigned char>(m_block[i * 4 + 0]) << 24);
		w[i] |= static_cast<unsigned int>(static_cast<unsigned char>(m_block[i * 4 + 1]) << 16);
		w[i] |= static_cast<unsigned int>(static_cast<unsigned char>(m_block[i * 4 + 2]) << 8);
		w[i] |= static_cast<unsigned int>(static_cast<unsigned char>(m_block[i * 4 + 3]));
	}

	for (size_t i = 16; i < 80; ++i)
	{
		w[i] = LeftRotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	}

	unsigned int a = m_h[0];
	unsigned int b = m_h[1];
	unsigned int c = m_h[2];
	unsigned int d = m_h[3];
	unsigned int e = m_h[4];

	for (size_t i = 0; i < 80; ++i)
	{
		unsigned int f;
		unsigned int k;

		if (i < 20)
		{
			f = (b & c) | (~b & d);
			k = 0x5A827999;
		}
		else if (i < 40)
		{
			f = b ^ c ^ d;
			k = 0x6ED9EBA1;
		}
		else if (i < 60)
		{
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
		}
		else
		{
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
		}

		const unsigned temp = LeftRotate(a, 5) + f + e + k + w[i];
		e = d;
		d = c;
		c = LeftRotate(b, 30);
		b = a;
		a = temp;
	}

	m_h[0] += a;
	m_h[1] += b;
	m_h[2] += c;
	m_h[3] += d;
	m_h[4] += e;
}

inline void Sha1::ProcessByteImpl(const std::byte byte)
{
	m_block[m_blockByteIndex++] = byte;

	if (m_blockByteIndex == 64)
	{
		m_blockByteIndex = 0;
		ProcessBlock();
	}
}

// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <span>
#include <string>


template<typename RangeType, typename RangeValue>
concept range_of = std::ranges::range<RangeType> && std::is_same_v<std::ranges::range_value_t<RangeType>, RangeValue>;

namespace PotatoAlert::Core {

class Sha1
{
	typedef unsigned int(DigestType)[5];

public:
	explicit Sha1()
	{
		Reset();
	}

	explicit Sha1(std::string_view s)
	{
		if (!s.empty())
			Process(s);
	}

	Sha1& Process(std::string_view s);

	operator std::string();
	std::string GetHash();

	void Reset();
	template<is_byte T>
	bool ProcessByte(T byte);
	template<is_byte T>
	bool ProcessBytes(std::span<T> bytes);

	template<is_byteRange R>
	bool ProcessBlock(const R& range)
	{
		auto begin = range.begin();
		auto end = range.end();
		while (begin != end)
		{
			if (!ProcessByte(*begin))
				return false;
			++begin;
		}
		return true;
	}

	void GetDigest(DigestType& digest);

private:
	void ProcessBlock();
	template<is_byte T>
	void ProcessByteImpl(T byte);

	Byte m_block[64] = {};
	DigestType m_h;
	size_t m_blockByteIndex;
	size_t m_bitCountLow;
	size_t m_bitCountHigh;
};

}  // namespace PotatoAlert::Core

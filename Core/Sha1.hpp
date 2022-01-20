// Copyright 2021 <github.com/razaqq>
#pragma once

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <span>
#include <string>


template<typename RangeType, typename RangeValue>
concept range_of = std::ranges::range<RangeType> && std::is_same_v<std::ranges::range_value_t<RangeType>, RangeValue>;

namespace PotatoAlert {

class Sha1
{
	typedef unsigned int(DigestType)[5];

public:
	explicit Sha1()
	{
		Reset();
	}

	explicit Sha1(const std::string& s)
	{
		if (!s.empty()) this->Process(s);
	}

	Sha1& Process(const std::string& s);

	operator std::string();
	std::string GetHash();

	void Reset();
	bool ProcessByte(std::byte byte);
	bool ProcessBytes(std::span<std::byte> bytes);

	template<range_of<std::byte> R>
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
	void ProcessByteImpl(std::byte byte);

	std::byte m_block[64] = {};
	DigestType m_h;
	size_t m_blockByteIndex;
	size_t m_bitCountLow;
	size_t m_bitCountHigh;
};

}  // namespace PotatoAlert

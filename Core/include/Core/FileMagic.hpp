// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <span>


namespace PotatoAlert::Core {

template<Byte... Magic>
struct FileMagic
{
	explicit FileMagic(std::span<const Byte>& data)
	{
		constexpr size_t size = sizeof...(Magic);
		if (data.size() < sizeof...(Magic))
		{
			m_valid = false;
			return;
		}
		const std::span<const Byte> dataMagic = Take(data, size);

		size_t i = 0;
		for (const Byte m : { Magic... })
		{
			if (dataMagic[i] != m)
			{
				m_valid = false;
				return;
			}
			i++;
		}
	}

	explicit operator bool() const
	{
		return m_valid;
	}

private:
	bool m_valid = true;
};

}  // namespace PotatoAlert::Core

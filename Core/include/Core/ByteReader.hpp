// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Result.hpp"
#include "Core/String.hpp"

#include <algorithm>
#include <cstdint>
#include <span>
#include <string>
#include <vector>


namespace PotatoAlert::Core {

enum class ReaderError
{
	InvalidSeek,
	OutOfBoundsSeek,
	OutOfBoundsRead,
};

static inline constexpr std::string_view ErrorMessage(ReaderError error)
{
	switch (error)
	{
		case ReaderError::InvalidSeek:
			return "Invalid seek origin";
		case ReaderError::OutOfBoundsSeek:
			return "Out of bounds seek";
		case ReaderError::OutOfBoundsRead:
			return "Out of bounds read";
	}
	return "Unknown Error";
}

enum class SeekOrigin
{
	Start,
	End,
	Current,
};

template<is_byte ByteType>
class ByteReader
{
private:
	std::span<const ByteType> m_data;
	size_t m_pos;

public:
	explicit ByteReader(std::span<const ByteType> data) : m_data(data), m_pos(0)
	{
	}

	void SetData(std::span<const ByteType> data)
	{
		m_data = data;
		m_pos = 0;
	}

	[[nodiscard]] size_t Size() const
	{
		return m_data.size() - m_pos;
	}

	[[nodiscard]] size_t Capacity() const
	{
		return m_data.size();
	}

	[[nodiscard]] size_t Position() const
	{
		return m_pos;
	}

	[[nodiscard]] bool Empty() const
	{
		return m_pos >= m_data.size() - 1;
	}

	const ByteType* RawPtr() const
	{
		return m_data.data();
	}

	Result<size_t, ReaderError> Seek(SeekOrigin from, int64_t offset)
	{
		switch (from)
		{
			case SeekOrigin::Start:
			{
				if (offset < (int64_t)m_data.size())
				{
					m_pos = offset;
					return m_pos;
				}
				return PA_ERROR(ReaderError::OutOfBoundsSeek);
			}
			case SeekOrigin::End:
			{
				if (offset <= 0 && -offset < (int64_t)m_data.size())
				{
					m_pos = m_data.size() + offset;
					return m_pos;
				}
				return PA_ERROR(ReaderError::OutOfBoundsSeek);
			}
			case SeekOrigin::Current:
			{
				if (offset < 0)
				{
					if (-offset <= (int64_t)m_pos)
					{
						m_pos += offset;
						return m_pos;
					}
					return PA_ERROR(ReaderError::OutOfBoundsSeek);
				}
				else
				{
					if (offset <= (int64_t)(m_data.size() - m_pos))
					{
						m_pos += offset;
						return m_pos;
					}
					return PA_ERROR(ReaderError::OutOfBoundsSeek);
				}
			}
		}
		return PA_ERROR(ReaderError::InvalidSeek);
	}

	// consumes n bytes from the buffer, stream pos will saturate at bounds, not overflow
	void Consume(size_t n)
	{
		m_pos = std::min(m_pos + n, m_data.size());
	}

	void Unconsume(size_t n)
	{
		m_pos = std::clamp((int64_t)m_pos - (int64_t)n, (int64_t)0, (int64_t)m_data.size());
	}

	Result<ByteType, ReaderError> ReadByte()
	{
		if (Empty())
			return PA_ERROR(ReaderError::OutOfBoundsRead);
		return m_data[m_pos++];
	}

	// reads buf.size() bytes and advances the position
	Result<size_t, ReaderError> Read(std::span<ByteType>& buf)
	{
		if (buf.size() < Size())
			return PA_ERROR(ReaderError::OutOfBoundsRead);
		std::copy(m_data.begin(), m_data.end(), buf.begin());
		m_pos += buf.size();
		return buf.size();
	}

	Result<std::span<const ByteType>, ReaderError> Read(size_t n)
	{
		if (n < Size())
			return PA_ERROR(ReaderError::OutOfBoundsRead);
		const std::span r = m_data.subspan(m_pos, n);
		m_pos += n;
		return r;
	}

	std::span<const ByteType> ReadUnsafe(size_t n)
	{
		const std::span r = m_data.subspan(m_pos, n);
		m_pos += n;
		return r;
	}

	size_t ReadToEnd(std::vector<ByteType>& buf)
	{
		if (Empty())
			return 0;
		const size_t toRead = m_data.size() - m_pos;
		buf.resize(toRead);
		std::copy(m_data.begin() + m_pos, m_data.end(), buf.begin());
		m_pos = m_data.size();
		return toRead;
	}

	template<typename T>
	Result<size_t, ReaderError> ReadTo(T& t)
	{
		if (Size() < sizeof(T))
			return PA_ERROR(ReaderError::OutOfBoundsRead);
		std::memcpy(&t, ReadUnsafe(sizeof(T)).data(), sizeof(T));
		return sizeof(T);
	}

	// reads n bytes into the string, string will be resized
	template<is_std_string TStr>
	Result<size_t, ReaderError> ReadToString(TStr& str, size_t n)
	{
		if (n > Size())
			return PA_ERROR(ReaderError::OutOfBoundsRead);
		str.resize(n);
		std::memcpy(str.data(), ReadUnsafe(n).data(), n);
		return n;
	}

	// reads str.size() bytes into the string
	Result<size_t, ReaderError> ReadToString(std::string_view str)
	{
		if (str.size() > Size())
			return PA_ERROR(ReaderError::OutOfBoundsRead);
		std::memcpy((void*)str.data(), ReadUnsafe(str.size()).data(), str.size());
		return str.size();
	}
};

}  // namespace PotatoAlert::Core

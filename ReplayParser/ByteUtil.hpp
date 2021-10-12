// Copyright 2021 <github.com/razaqq>
#pragma once

#include <cassert>
#include <span>
#include <string>


namespace PotatoAlert::ReplayParser
{

constexpr std::span<std::byte> Take(std::span<std::byte>& data, size_t n)
{
	assert(data.size() >= n);  // TODO: make this a check instead of an assert
	const std::span r = data.subspan(0, n);
	data = data.subspan(n);
	return r;
}

template<typename T>
static bool TakeInto(std::span<std::byte>& data, T&& dst)
{
	if (data.size() >= sizeof(dst))
	{
		std::memcpy(&dst, Take(data, sizeof(dst)).data(), sizeof(dst));
		return true;
	}
	/*
	if (std::is_trivially_copyable<T>::value)
	{
		if (s.size() >= sizeof(dst))
		{
			std::memcpy(&dst, Take(s, sizeof(dst)).data(), sizeof(dst));
			return true;
		}
	}
	*/
	
	return false;
}

static bool TakeString(std::span<std::byte>& data, std::string& str, size_t size)
{
	if (data.size() >= size)
	{
		str.resize(size);
		std::memcpy(str.data(), Take(data, size).data(), size);
		return true;
	}
	return false;
}

static bool TakeWString(std::span<std::byte>& data, std::wstring& str, size_t size)
{
	if (data.size() >= size)
	{
		str.resize(size);
		std::memcpy(str.data(), Take(data, size).data(), size);
		return true;
	}
	return false;
}

template<typename... Ts>
static std::array<std::byte, sizeof...(Ts)> MakeBytes(Ts&&... args) noexcept
{
	return { std::byte(std::forward<Ts>(args))... };
}

}  // namespace PotatoAlert::ReplayParser

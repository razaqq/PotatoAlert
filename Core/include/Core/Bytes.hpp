// Copyright 2021 <github.com/razaqq>
#pragma once

#include <cassert>
#include <iomanip>
#include <ios>
#include <span>
#include <sstream>
#include <string>


namespace PotatoAlert::Core {

template<typename T>
concept is_byte = sizeof(T) == 1;

template<is_byte T>
constexpr std::span<T> Take(std::span<T>& data, size_t n)
{
	assert(data.size() >= n);  // TODO: make this a check instead of an assert
	const std::span r = data.subspan(0, n);
	data = data.subspan(n);
	return r;
}

template<is_byte TIn, typename TVal>
static bool TakeInto(std::span<TIn>& data, TVal&& dst)  // requires std::is_fundamental_v<std::decay_t<T>> || std::is_fundamental_v<std::decay_t<std::underlying_type<T>>>
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

template<is_byte T>
std::string FormatBytes(std::span<T> data)
{
	std::ostringstream result;
	for (auto begin = data.begin(); begin != data.end();)
	{
		result << "0x" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << std::to_integer<int>(*begin);
		++begin;
		if (begin != data.end())
		{
			result << " ";
		}
	}

	return result.str();
}

template<typename T>
concept is_string = std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>;

template<is_byte TIn, is_string TStr>
static bool TakeString(std::span<TIn>& data, TStr& str, size_t size)
{
	if (data.size() >= size)
	{
		str.resize(size);
		std::memcpy(str.data(), Take(data, size).data(), size);
		return true;
	}
	return false;
}

template<is_byte TOut, typename... Ts> requires std::conjunction_v<std::is_integral<Ts>...>
static std::array<TOut, sizeof...(Ts)> MakeBytes(Ts&&... args) noexcept
{
	return { TOut(std::forward<Ts>(args))... };
}

}  // namespace PotatoAlert::Core

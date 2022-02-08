// Copyright 2021 <github.com/razaqq>
#pragma once

#include <type_traits>


namespace PotatoAlert::Core {

template<typename T>
constexpr bool HasFlag(T flags, T flag)
{
	return (flags & flag) != static_cast<T>(0);
}

}  // namespace PotatoAlert::Core


#define DEFINE_FLAGS(flags) \
	constexpr flags operator| [[maybe_unused]] (flags a, flags b) \
	{ \
		typedef std::underlying_type_t<flags> T; \
		return static_cast<flags>(static_cast<T>(a) | static_cast<T>(b)); \
	} \
	constexpr flags operator& [[maybe_unused]] (flags a, flags b) \
	{ \
		typedef std::underlying_type_t<flags> T; \
		return static_cast<flags>(static_cast<T>(a) & static_cast<T>(b)); \
	} \
	constexpr flags operator~ [[maybe_unused]] (flags a) \
	{ \
		typedef std::underlying_type_t<flags> T; \
		return static_cast<flags>(~static_cast<T>(a)); \
	}

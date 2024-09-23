// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Core/TypeTraits.hpp"

#include <concepts>
#include <ranges>
#include <string>


namespace PotatoAlert::Core {

template<typename T, typename... Ts>
concept any_of = is_any_of_v<T, Ts...>;

template<typename T>
concept character = std::integral<T> && any_of<T, char, wchar_t, char8_t, char16_t, char32_t>;

template<typename T>
concept is_std_string = std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>;

template<typename T>
concept is_string = std::ranges::contiguous_range<T> && character<std::ranges::range_value_t<T>>;

}  // namespace PotatoAlert::Core

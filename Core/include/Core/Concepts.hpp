// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Core/TypeTraits.hpp"

#include <concepts>


namespace PotatoAlert::Core {

template<typename T, typename... Ts>
concept any_of = is_any_of_v<T, Ts...>;

template<typename T>
concept character = std::integral<T> && any_of<T, char, wchar_t, char8_t, char16_t, char32_t>;

}  // namespace PotatoAlert::Core

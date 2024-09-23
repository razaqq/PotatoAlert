// Copyright 2023 <github.com/razaqq>
#pragma once

#include <type_traits>


namespace PotatoAlert::Core {

template<typename...>
inline constexpr bool always_false = false;

template<typename...>
inline constexpr bool always_true = true;

template<bool Value, typename...>
inline constexpr bool bool_value = Value;

template<typename T, typename... Ts>
inline constexpr bool is_any_of_v = (std::is_same_v<T, Ts> || ...);

template<typename T, typename... Ts>
using is_any_of = std::bool_constant<is_any_of_v<T, Ts...>>;

}

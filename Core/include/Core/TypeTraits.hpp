// Copyright 2023 <github.com/razaqq>
#pragma once

namespace PotatoAlert::Core {

template<typename...>
inline constexpr bool always_false = false;

template<typename...>
inline constexpr bool always_true = true;

template<bool Value, typename...>
inline constexpr bool bool_value = Value;

}

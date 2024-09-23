// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Preprocessor.hpp"

#include <concepts>
#include <type_traits>
#include <utility>


namespace PotatoAlert::Core {

namespace Detail {

template<std::invocable Callable>
class Defer
{
public:
	[[nodiscard]] Defer(std::convertible_to<Callable> auto&& callback)
		: m_callback(std::move(callback))
	{
	}

	Defer(const Defer&) = delete;
	Defer& operator=(const Defer&) = delete;

	~Defer()
	{
		std::move(m_callback)();
	}
private:
	Callable m_callback;
};

template<typename Callable>
Defer(Callable&&) -> Defer<std::remove_reference_t<Callable>>;

}  // namespace Detail


template<typename Callable>
[[nodiscard]] auto MakeDefer(std::invocable auto&& callable)
{
	return Detail::Defer(std::forward<std::decay_t<Callable>>(callable));
}

#define PA_DEFER \
	::PotatoAlert::Core::Detail::Defer PA_ANONYMOUS(pa_detail_defer) = [&]() noexcept -> void

}  // namespace PotatoAlert::Core

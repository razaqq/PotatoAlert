// Copyright 2022 <github.com/razaqq>
#pragma once


namespace PotatoAlert::Core {

template<typename Callable>
class Defer
{
public:
	[[nodiscard]] explicit Defer(Callable&& callback) : m_callback(callback), m_engaged(true)
	{
		static_assert(std::is_convertible_v<Callable, std::function<void()>>,
					  "Callback needs to be convertible to std::function<void()>");
	}

	Defer(Defer&& defer) noexcept : m_callback(std::move(defer.m_callback)), m_engaged(defer.m_engaged)
	{
		defer.Release();
	}

	Defer(const Defer&) = delete;
	Defer& operator=(Defer&&) = delete;
	Defer& operator=(const Defer&) = delete;

	void Release()
	{
		m_engaged = false;
	}

	~Defer()
	{
		if (m_engaged)
			m_callback();
	}
private:
	Callable m_callback;
	bool m_engaged;
};


template<typename Callable>
[[nodiscard]] constexpr Defer<std::decay_t<Callable>> MakeDefer(Callable&& callable)
{
	return Defer(std::forward<std::decay_t<Callable>>(callable));
}

}  // namespace PotatoAlert::Core

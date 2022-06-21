// Copyright 2021 <github.com/razaqq>
#pragma once

#define PA_SINGLETON(Class)                  \
	static Class& Instance()                 \
	{                                        \
		static Class c;                      \
		return c;                            \
	}                                        \
	Class(const Class&) = delete;            \
	Class(Class&&) noexcept = delete;        \
	Class& operator=(const Class&) = delete; \
	Class& operator=(Class&&) noexcept = delete

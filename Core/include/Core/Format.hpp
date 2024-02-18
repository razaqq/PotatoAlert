// Copyright 2023 <github.com/razaqq>
#pragma once

#if !defined(PA_USE_STD_FORMAT) && !defined(PA_USE_FMT_FORMAT)
	#define PA_USE_FMT_FORMAT
#endif

#ifdef PA_USE_STD_FORMAT
	#define FORMAT std
	#include <format>
#endif

#ifdef PA_USE_FMT_FORMAT
	#define FORMAT fmt
	#include <fmt/format.h>
	#include <fmt/chrono.h>
	#include <fmt/ranges.h>
	#include <fmt/std.h>
	#include <fmt/xchar.h>
#endif

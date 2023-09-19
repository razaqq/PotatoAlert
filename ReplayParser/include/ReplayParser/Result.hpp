// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Format.hpp"
#include "Core/Result.hpp"

#include <expected>
#include <string>

namespace PotatoAlert::ReplayParser {

using ReplayError = std::string;
template<typename T>
using ReplayResult = Core::Result<T, ReplayError>;

template<class... T>
inline std::unexpected<ReplayError> MakeReplayError(std::string_view fmt, T&&... args)
{
	return std::unexpected(ReplayError(fmt::format(fmt, args...)));
}

#define PA_REPLAY_ERROR(...) (::std::unexpected(::PotatoAlert::ReplayParser::ReplayError(fmt::format(__VA_ARGS__))))

}  // namespace PotatoAlert::ReplayParser

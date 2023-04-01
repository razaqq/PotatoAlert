// Copyright 2023 <github.com/razaqq>
#pragma once

#include <Core/Result.hpp>

#include <expected>
#include <format>
#include <string>

namespace PotatoAlert::ReplayParser {

using ReplayError = std::string;
template<typename T>
using ReplayResult = Core::Result<T, ReplayError>;
#ifdef __INTELLISENSE__
#define PA_REPLAY_ERROR(...) (std::format(__VA_ARGS__))
#else
#define PA_REPLAY_ERROR(...) (::std::unexpected(::PotatoAlert::ReplayParser::ReplayError(std::format(__VA_ARGS__))))
#endif

}  // namespace PotatoAlert::ReplayParser

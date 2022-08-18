// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"

#include <span>
#include <vector>


namespace PotatoAlert::Core::Zlib {

std::vector<Byte> Inflate(std::span<const Byte> in, bool hasHeader = true);

}  // namespace PotatoAlert::Core::Zlib

// Copyright 2021 <github.com/razaqq>
#pragma once

#include <span>
#include <vector>


namespace PotatoAlert::Core::Zlib {

std::vector<std::byte> Inflate(std::span<const std::byte> in);

}  // namespace PotatoAlert::Core::Zlib
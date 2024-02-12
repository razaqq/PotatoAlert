// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Format.hpp"
#include "Core/Result.hpp"

#include <expected>
#include <filesystem>
#include <string>

#include <tinyxml2.h>


namespace PotatoAlert::Core {

using XmlError = std::string;
template<typename T>
using XmlResult = Result<T, XmlError>;
#define PA_XML_ERROR(...) (::std::unexpected(::PotatoAlert::Core::XmlError(fmt::format(__VA_ARGS__))))

XmlResult<void> LoadXml(tinyxml2::XMLDocument& doc, const std::filesystem::path& xmlPath);

}  // namespace PotatoAlert::Core

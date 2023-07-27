// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Log.hpp"
#include "Core/Result.hpp"

#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <type_traits>

#include <tinyxml2.h>
#include <stdio.h>


namespace PotatoAlert::Core {

using XmlError = std::string;
template<typename T>
using XmlResult = Result<T, XmlError>;
#define PA_XML_ERROR(...) (::std::unexpected(::PotatoAlert::Core::XmlError(std::format(__VA_ARGS__))))

template<typename>
errno_t WfOpen(FILE** stream, const wchar_t* fileName, const wchar_t* mode)
{
	return _wfopen_s(stream, fileName, mode);
}

template<typename T = void>
static inline XmlResult<void> LoadXml(tinyxml2::XMLDocument& doc, const std::filesystem::path& xmlPath)
{
	if constexpr (std::is_same_v<std::filesystem::path::value_type, char>)
	{
		const tinyxml2::XMLError err = doc.LoadFile(xmlPath.string().c_str());
		if (err != tinyxml2::XML_SUCCESS)
		{
			return PA_XML_ERROR("{}", doc.ErrorStr());
		}
		return {};
	}
	else
	{
		const std::wstring path = xmlPath.native();
		FILE* file = nullptr;
		if (_wfopen_s(&file, path.c_str(), L"rb") == 0)
		{
			const tinyxml2::XMLError err = doc.LoadFile(file);
			fclose(file);
			if (err != tinyxml2::XML_SUCCESS)
			{
				return PA_XML_ERROR("{}", doc.ErrorStr());
			}
			return {};
		}
		else
		{
			return PA_XML_ERROR("Failed to open xml file.");
		}
	}
}

}  // namespace PotatoAlert::Core

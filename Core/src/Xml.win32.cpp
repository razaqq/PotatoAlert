// Copyright 2024 <github.com/razaqq>

#include "Core/Xml.hpp"

#include <filesystem>
#include <string>

#include <stdio.h>


PotatoAlert::Core::XmlResult<void> PotatoAlert::Core::LoadXml(tinyxml2::XMLDocument& doc, const std::filesystem::path& xmlPath)
{
	static_assert(std::is_same_v<std::filesystem::path::value_type, wchar_t>, "std::filesystem::path::value_type != wchar_t");
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

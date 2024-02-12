// Copyright 2024 <github.com/razaqq>

#include "Core/Xml.hpp"

#include <tinyxml2.h>

#include <filesystem>


PotatoAlert::Core::XmlResult<void> PotatoAlert::Core::LoadXml(tinyxml2::XMLDocument& doc, const std::filesystem::path& xmlPath)
{
	static_assert(std::is_same_v<std::filesystem::path::value_type, char>, "std::filesystem::path::value_type != wchar_t");
	const tinyxml2::XMLError err = doc.LoadFile(xmlPath.string().c_str());
	if (err != tinyxml2::XML_SUCCESS)
	{
		return PA_XML_ERROR("{}", doc.ErrorStr());
	}
	return {};
}

// Copyright 2021 <github.com/razaqq>
#include "Json.hpp"


using PotatoAlert::Core::Json;

Json Json::Parse(const std::string& raw)
{
	json j;
	sax_no_exception sax(j);
	if (!json::sax_parse(raw, &sax))
	{
		return Json(nullptr);
	}

	return Json(j);
}

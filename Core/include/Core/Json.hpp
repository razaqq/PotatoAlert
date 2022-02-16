// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Log.hpp"

#include <string>


#define JSON_TRY_USER if (true)
#define JSON_CATCH_USER(exception) if (false)
#define JSON_THROW_USER(exception)                                                                       \
	{                                                                                                    \
		LOG_ERROR("Error in {}:{} (function {}) - {}", __FILE__, __LINE__, __PRETTY_FUNCTION__,          \
				  (exception).what());                                                                   \
		std::abort();                                                                                    \
	}
#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
using json = nlohmann::json;


class sax_no_exception : public nlohmann::detail::json_sax_dom_parser<json>
{
public:
	explicit sax_no_exception(json& j) : nlohmann::detail::json_sax_dom_parser<json>(j, false) {}

	static bool parse_error(size_t position, const std::string& lastToken, const json::exception& ex)
	{
		LOG_ERROR("JSON Parse Error at input byte {}. {}. Last read: \"{}\"", position, ex.what(), lastToken);
		return false;
	}
};

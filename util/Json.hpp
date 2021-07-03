// Copyright 2021 <github.com/razaqq>
#pragma once

#include <string>
#include "Log.hpp"



#define JSON_TRY_USER if (true)
#define JSON_CATCH_USER(exception) if (false)
#define JSON_THROW_USER(exception)           \
	{                                        \
		LOG_ERROR("Error in {}:{} (function {}) - {} (ID: {})", __FILE__, __LINE__, __PRETTY_FUNCTION__, \
		(exception).what(), (exception).id); \
		std::abort();                        \
	}
#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
using json = nlohmann::json;


class sax_no_exception : public nlohmann::detail::json_sax_dom_parser<json>
{
public:
	explicit sax_no_exception(json& j) : nlohmann::detail::json_sax_dom_parser<json>(j, false) {}

	static bool parse_error(std::size_t position, const std::string& lastToken, const json::exception& ex)
	{
		LOG_ERROR("JSON Parse Error at input byte {}. {}. Last read: \"{}\"", position, ex.what(), lastToken);
		return false;
	}
};

/*
// add support for std::optional<T>
namespace nlohmann {

template <typename T>
struct [[maybe_unused]] adl_serializer<std::optional<T>> {
	[[maybe_unused]] static void from_json(const json& j, std::optional<T>& opt)
	{
		if (j.is_null())
			opt = std::nullopt;
		else
			opt = j.get<T>();
	}
};

}  // namespace nlohmann
*/

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

#define NLOHMANN_JSON_SERIALIZE_ENUM_PAIRS(ENUM_TYPE, PAIRS)                                 \
	template<typename BasicJsonType>                                                         \
	inline void to_json(BasicJsonType& j, const ENUM_TYPE& e)                                \
	{                                                                                        \
		static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");       \
		auto it = std::find_if(std::begin(PAIRS), std::end(PAIRS),                           \
							[e](const std::pair<ENUM_TYPE, BasicJsonType>& ej_pair) -> bool  \
							{                                                                \
								return ej_pair.first == e;                                   \
							});                                                              \
		j = ((it != std::end(PAIRS)) ? it : std::begin(PAIRS))->second;                      \
	}                                                                                        \
	template<typename BasicJsonType>                                                         \
	inline void from_json(const BasicJsonType& j, ENUM_TYPE& e)                              \
	{                                                                                        \
		static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");       \
		auto it = std::find_if(std::begin(PAIRS), std::end(PAIRS),                           \
							[&j](const std::pair<ENUM_TYPE, BasicJsonType>& ej_pair) -> bool \
							{                                                                \
								return ej_pair.second == j;                                  \
							});                                                              \
		e = ((it != std::end(PAIRS)) ? it : std::begin(PAIRS))->first;                       \
	}

#define NLOHMANN_JSON_SERIALIZE_ENUM_MAP(ENUM_TYPE, MAP)                                     \
	template<typename BasicJsonType>                                                         \
	inline void to_json(BasicJsonType& j, const ENUM_TYPE& e)                                \
	{                                                                                        \
		static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");       \
		if ((MAP).contains(e))                                                               \
		{                                                                                    \
			j = (MAP).at(e);                                                                 \
		}                                                                                    \
		else                                                                                 \
		{                                                                                    \
			j = (MAP).begin()->second;                                                       \
		}                                                                                    \
	}                                                                                        \
	template<typename BasicJsonType>                                                         \
	inline void from_json(const BasicJsonType& j, ENUM_TYPE& e)                              \
	{                                                                                        \
		static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");       \
		auto it = std::find_if(std::begin(MAP), std::end(MAP),                               \
							[&j](const std::pair<ENUM_TYPE, BasicJsonType>& ej_pair) -> bool \
							{                                                                \
								return ej_pair.second == j;                                  \
							});                                                              \
		e = ((it != std::end(MAP)) ? it : std::begin(MAP))->first;                           \
	}

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

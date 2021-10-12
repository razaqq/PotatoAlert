// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Log.hpp"

#include <string>


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

	static bool parse_error(size_t position, const std::string& lastToken, const json::exception& ex)
	{
		LOG_ERROR("JSON Parse Error at input byte {}. {}. Last read: \"{}\"", position, ex.what(), lastToken);
		return false;
	}
};

namespace PotatoAlert {

class Json
{
public:
	explicit Json(json j) : m_json(std::move(j)) {}
	static Json Parse(const std::string& raw);

	Json(Json&& src) noexcept
	{
		m_json = std::exchange(src.m_json, nullptr);
	}

	Json(const Json&) = delete;

	Json& operator=(Json&& src) noexcept
	{
		m_json = std::exchange(src.m_json, nullptr);
		return *this;
	}

	Json& operator=(const Json&) = delete;

	~Json()
	{
		m_json = nullptr;
	}

	[[nodiscard]] bool HasError() const { return m_hasError; }

	explicit operator bool() const
	{
		return m_json != nullptr;
	}

	bool operator==(decltype(nullptr)) const
	{
		return m_json == nullptr;
	}

	bool operator!=(decltype(nullptr)) const
	{
		return m_json != nullptr;
	}

private:
	json m_json;
	bool m_hasError = false;
};

}  // namespace PotatoAlert

// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Concepts.hpp"
#include "Core/Format.hpp"
#include "Core/Result.hpp"

#include <glaze/glaze.hpp>

#include <expected>
#include <string>


namespace PotatoAlert::Core {

using JsonError = std::string;
template<typename T>
using JsonResult = Result<T, JsonError>;
#define PA_JSON_ERROR(...) (::std::unexpected(::PotatoAlert::Core::JsonError(fmt::format(__VA_ARGS__))))


namespace Json {

template<is_number T>
JsonResult<T> Get(const glz::json_t& j)
{
	if (!j.is_number())
	{
		return PA_JSON_ERROR("Json value is not a number");
	}

	return j.as<T>();
}

template<is_bool T>
JsonResult<T> Get(const glz::json_t& j)
{
	if (!j.is_boolean())
	{
		return PA_JSON_ERROR("Json value is not a bool");
	}

	return j.get_boolean();
}

template<typename T>
	requires(std::is_same_v<T, std::string>)
JsonResult<T> Get(const glz::json_t& j)
{
	if (!j.is_string())
	{
		return PA_JSON_ERROR("Json value is not a string");
	}

	return j.get_string();
}

template<typename T, glz::string_literal Pointer>
JsonResult<T> GetPointer(std::string_view str)
{
	const auto res = glz::get_as_json<T, Pointer>(str);
	if (!res)
	{
		return std::unexpected(JsonError(glz::format_error(res, str)));
	}
	return *res;
}

template<typename T, auto Opts = glz::opts{}>
JsonResult<T> Read(std::string_view json)
{
	T t;
	const glz::error_ctx res = glz::read<Opts, T>(t, json);
	if (res)
	{
		return std::unexpected(JsonError(glz::format_error(res, json)));
	}
	return t;
}

template<auto Opts = glz::opts{}, typename T>
JsonResult<std::string> Write(const T& obj)
{
	std::string buffer;
	const glz::error_ctx res = glz::write<Opts>(obj, buffer);
	if (res)
	{
		return std::unexpected(JsonError(glz::format_error(res)));
	}
	return buffer;
}

}  // namespace Json

}  // namespace PotatoAlert::Core

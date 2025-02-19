// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Format.hpp"
#include "Core/Result.hpp"
#include "Core/String.hpp"

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson {
	typedef ::std::size_t SizeType;
}
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>

#include <array>
#include <cstdint>
#include <expected>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef WIN32
	#undef GetObject
#endif


namespace PotatoAlert::Core {

using JsonError = std::string;
template<typename T>
using JsonResult = Result<T, JsonError>;
#define PA_JSON_ERROR(...) (::std::unexpected(::PotatoAlert::Core::JsonError(fmt::format(__VA_ARGS__))))

[[maybe_unused]] static rapidjson::GenericStringRef<char> ToRef(std::string_view str)
{
	return { str.data(), str.size() };
}

static inline JsonResult<rapidjson::Document> ParseJson(std::string_view json)
{
	rapidjson::Document doc;
	doc.Parse(json.data(), json.size());

	if (doc.HasParseError())
	{
		return PA_JSON_ERROR("Json parse error: {} ({})\n", GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
	}
	return doc;
}

template<typename T>
static inline JsonResult<T> FromJson(std::string_view json)
{
	rapidjson::Document doc;
	doc.Parse(json.data(), json.size());

	if (doc.HasParseError())
	{
		return PA_JSON_ERROR("Json parse error: {} ({})\n", GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
	}
	return FromJson<T>(doc);
}

template<typename T>
concept is_bool = std::is_same_v<T, bool>;

template<typename T>
concept is_number = (std::is_floating_point_v<T> || std::is_integral_v<T>) && !is_bool<T>;

static_assert(std::is_integral_v<bool>);

template<typename T>
concept is_primitive_serializable = is_bool<T> || is_number<T> || is_std_string<T>;

template<typename T, typename OutputStream = rapidjson::StringBuffer>
concept is_serializable = is_primitive_serializable<T> || requires (T a, rapidjson::Writer<OutputStream> writer)
{
	ToJson(writer, a);
};

template<typename T>
concept is_deserializable = is_primitive_serializable<T> ||
	requires(T a, const rapidjson::Value& json) { FromJson<T>(json); } ||
	requires(T a, const rapidjson::Value& json) { FromJson(json, a); };

template<typename OutputStream = rapidjson::StringBuffer>
static inline bool ToJson(rapidjson::Writer<OutputStream>& writer, bool value)
{
	return writer.Bool(value);
}

template<is_number T, typename OutputStream = rapidjson::StringBuffer>
static inline bool ToJson(rapidjson::Writer<OutputStream>& writer, T value)
{
	using D = std::decay_t<T>;
	if constexpr (std::is_same_v<D, int32_t>)
	{
		return writer.Int(value);
	}
	else if constexpr (std::is_same_v<D, uint32_t>)
	{
		return writer.Uint(value);
	}
	else if constexpr (std::is_same_v<D, int64_t>)
	{
		return writer.Int64(value);
	}
	else if constexpr (std::is_same_v<D, uint64_t>)
	{
		return writer.Uint64(value);
	}
	else if constexpr (std::is_same_v<D, float>)
	{
		return writer.Double(value);
	}
	else if constexpr (std::is_same_v<D, double>)
	{
		return writer.Double(value);
	}

	return false;
}

template<typename OutputStream = rapidjson::StringBuffer>
static inline bool ToJson(rapidjson::Writer<OutputStream>& writer, std::string_view str)
{
	return writer.String(str.data());
}

template<is_std_string T, typename OutputStream = rapidjson::StringBuffer>
static inline bool ToJson(rapidjson::Writer<OutputStream>& writer, const T& str)
	requires(str.c_str())
{
	return writer.String(str.c_str());
}

template<is_std_string T, typename OutputStream = rapidjson::StringBuffer>
static inline bool ToJson(rapidjson::Writer<OutputStream>& writer, const T& str)
{
	return writer.String(str);
}

template<is_serializable Key, is_serializable Value>
static inline bool ToJson(rapidjson::Writer<rapidjson::StringBuffer>& writer, const std::unordered_map<Key, Value>& map)
{
	bool start = writer.StartObject();
	for (const auto& [key, value] : map)
	{
		if (!ToJson(writer, key) || !ToJson(writer, value))
			return false;
	}
	bool end = writer.EndObject();

	return start && end;
}

template<is_std_string T>
static inline T FromJson(const rapidjson::Value& v)
{
	T str(v.GetString(), v.GetStringLength());
	return str;
}

template<is_bool T>
static inline T FromJson(const rapidjson::Value& v)
{
	return v.GetBool();
}

template<is_number T>
static inline T FromJson(const rapidjson::Value& v)
{
	// TODO: technically we have to check for i8 and i16
	using D = std::decay_t<T>;
	if constexpr (std::is_same_v<D, int8_t>)
	{
		return static_cast<int8_t>(v.GetInt());
	}
	if constexpr (std::is_same_v<D, uint8_t>)
	{
		return static_cast<uint8_t>(v.GetUint());
	}
	if constexpr (std::is_same_v<D, int16_t>)
	{
		return static_cast<int16_t>(v.GetInt());
	}
	if constexpr (std::is_same_v<D, uint16_t>)
	{
		return static_cast<uint16_t>(v.GetUint());
	}
	if constexpr (std::is_same_v<D, int32_t>)
	{
		return v.GetInt();
	}
	else if constexpr (std::is_same_v<D, uint32_t>)
	{
		return v.GetUint();
	}
	else if constexpr (std::is_same_v<D, int64_t>)
	{
		return v.GetInt64();
	}
	else if constexpr (std::is_same_v<D, uint64_t>)
	{
		return v.GetUint64();
	}
	else if constexpr (std::is_same_v<D, float>)
	{
		return v.GetDouble();
	}
	else if constexpr (std::is_same_v<D, double>)
	{
		return v.GetDouble();
	}

	T t;
	return t;
}

static inline rapidjson::Value ToJson(std::string_view str, rapidjson::MemoryPoolAllocator<>& allocator)
{
	return { str.data(), str.size(), allocator };
}

static inline rapidjson::Value ToJson(bool v, [[maybe_unused]] rapidjson::MemoryPoolAllocator<>& allocator)
{
	return rapidjson::Value(v);
}

template<is_number T>
static inline rapidjson::Value ToJson(T t, [[maybe_unused]] rapidjson::MemoryPoolAllocator<>& allocator)
{
	rapidjson::Value value;

	using D = std::decay_t<T>;
	if constexpr (std::is_same_v<D, int8_t>)
	{
		value = value.SetInt(t);
	}
	if constexpr (std::is_same_v<D, uint8_t>)
	{
		value = value.SetUint(t);
	}
	if constexpr (std::is_same_v<D, int16_t>)
	{
		value = value.SetInt(t);
	}
	if constexpr (std::is_same_v<D, uint16_t>)
	{
		value = value.SetUint(t);
	}
	if constexpr (std::is_same_v<D, int32_t>)
	{
		value = value.SetInt(t);
	}
	else if constexpr (std::is_same_v<D, uint32_t>)
	{
		value = value.SetUint(t);
	}
	else if constexpr (std::is_same_v<D, int64_t>)
	{
		value = value.SetInt64(t);
	}
	else if constexpr (std::is_same_v<D, uint64_t>)
	{
		value = value.SetUint64(t);
	}
	else if constexpr (std::is_same_v<D, float>)
	{
		value = value.SetFloat(t);
	}
	else if constexpr (std::is_same_v<D, double>)
	{
		value = value.SetDouble(t);
	}
	else
	{
		value = value.Set(t);
	}

	return value;
}

template<is_deserializable Key, is_deserializable Value>
static inline JsonResult<void> FromJsonImpl(std::unordered_map<Key, Value>& map, const rapidjson::Value& key, const rapidjson::Value& value)
{
	Key k;
	if constexpr (std::is_enum_v<Key>)
	{
		FromJson(key, k);
	}
	else
	{
		k = FromJson<Key>(key);
	}

	if constexpr (is_primitive_serializable<Value>)
	{
		if (!map.try_emplace(k, FromJson<Value>(value)).second)
			return PA_JSON_ERROR("Failed to emplace in std::unordered_map");
	}
	else
	{
		Value v;
		PA_TRYV(FromJson(value, v));
		if (!map.try_emplace(k, v).second)
			return PA_JSON_ERROR("Failed to emplace in std::unordered_map");
	}

	return {};
}

template<is_deserializable Key, is_deserializable Value>
static inline JsonResult<void> FromJson(const rapidjson::Value& j, std::unordered_map<Key, Value>& map)
{
	if (j.IsObject())
	{
		for (const auto& m : j.GetObject())
		{
			PA_TRYV(FromJsonImpl(map, m.name, m.value));
		}
		return {};
	}
	else if (j.IsArray())
	{
		for (const auto& m : j.GetArray())
		{
			if (m.Size() != 2)
			{
				return PA_JSON_ERROR("JSON for std::unordered_map is type array but size was != 2");
			}

			PA_TRYV(FromJsonImpl(map, m[0], m[1]));
		}
		return {};
	}
	return PA_JSON_ERROR("JSON for std::unordered_map is neither of type object nor array");
}

template<is_deserializable Value, size_t Size>
static inline bool FromJson(const rapidjson::Value& j, std::array<Value, Size>& arr)
{
	if (!j.IsArray())
		return false;

	const auto value = j.GetArray();
	size_t count = value.Size();
	if (count > arr.size())
		return false;

	for (size_t i = 0; i < count; i++)
	{
		if constexpr (is_primitive_serializable<Value>)
		{
			arr[i] = FromJson<Value>(j[i]);
		}
		else
		{
			Value v;
			if (!FromJson(j[i], v))
				return false;
			arr[i] = v;
		}
	}
	return true;
}

template<is_deserializable Value>
static inline bool FromJson(const rapidjson::Value& j, std::vector<Value>& vec)
{
	if (!j.IsArray())
		return false;

	const auto value = j.GetArray();
	size_t size = value.Size();
	vec.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		if constexpr (is_primitive_serializable<Value>)
		{
			vec.emplace_back(FromJson<Value>(j[i]));
		}
		else
		{
			Value v;
			if (!FromJson(j[i], v))
				return false;
			vec.emplace_back(v);
		}
	}
	return true;
}

template<is_deserializable Value>
static inline bool FromJson(const rapidjson::Value& j, std::unordered_set<Value>& set)
{
	if (!j.IsArray())
		return false;

	const auto value = j.GetArray();
	size_t size = value.Size();
	set.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		if constexpr (is_primitive_serializable<Value>)
		{
			set.emplace(FromJson<Value>(j[i]));
		}
		else
		{
			Value v;
			if (!FromJson(j[i], v))
				return false;
			set.emplace(v);
		}
	}
	return true;
}

template<class, template<class...> class>
inline constexpr bool is_specialization = false;

template<template<class...> class T, class... Args>
inline constexpr bool is_specialization<T<Args...>, T> = true;

template<class T>
concept is_vector = is_specialization<T, std::vector>;

template<typename T>
concept is_deserializable_vec = is_vector<T> && is_primitive_serializable<typename T::value_type>;

template<is_deserializable_vec T>
static inline bool FromJson(const rapidjson::Value& j, T& vec)
{
	using ValueType = typename T::value_type;

	if (!j.IsObject())
		return false;

	const auto obj = j.GetObject();
	vec.reserve(obj.MemberCount());

	for (const auto& m : obj)
	{
		vec.emplace_back(Core::FromJson<ValueType>(m.value));
	}
	return true;
}

template<typename T>
static inline JsonResult<T> FromJson(const rapidjson::Value& json, std::string_view key)
{
	using D = std::decay_t<T>;

	if (!json.HasMember(key.data()))
		return PA_JSON_ERROR("Json object has no key '{}'", key);

	if constexpr (is_bool<D>)
	{
		if (!json[key.data()].IsBool())
			return PA_JSON_ERROR("Json value '{}' is not of type 'bool'", key);
	}
	else if constexpr (is_number<D>)
	{
		if (!json[key.data()].IsNumber())
			return PA_JSON_ERROR("Json value '{}' is not of type '{}'", key, typeid(D).name());
	}
	else if constexpr (is_std_string<D>)
	{
		if (!json[key.data()].IsString())
			return PA_JSON_ERROR("Json value '{}' is not of type '{}'", key, typeid(D).name());
	}
	else
	{
		return PA_JSON_ERROR("Unsupported json value '{}'", typeid(D).name());
	}
	return Core::FromJson<D>(json[key.data()]);
}

template<is_primitive_serializable T>
static inline JsonResult<void> FromJson(const rapidjson::Value& json, std::string_view key, T& v)
{
	using D = std::decay_t<T>;

	if (!json.HasMember(key.data()))
		return PA_JSON_ERROR("Json object has no key '{}'", key);

	if constexpr (is_std_string<D>)
	{
		if (!json[key.data()].IsString())
			return PA_JSON_ERROR("Json value '{}' is not of type '{}'", key, typeid(D).name());
	}
	else if constexpr (is_number<D>)
	{
		if (!json[key.data()].IsNumber())
			return PA_JSON_ERROR("Json value '{}' is not of type '{}'", key, typeid(D).name());
	}
	else
	{
		if (!json.Is<D>())
			return PA_JSON_ERROR("Json value '{}' is not of type '{}'", key, typeid(D).name());
	}
	
	v = Core::FromJson<T>(json[key.data()]);
	return {};
}

template<is_deserializable T>
static inline JsonResult<void> FromJson(const rapidjson::Value& json, std::string_view key, T& v)
{
	using D = std::decay_t<T>;

	if (!json.HasMember(key.data()))
		return PA_JSON_ERROR("Json object has no key '{}'", key);

	if (!Core::FromJson<D>(json[key.data()], v))
		return PA_JSON_ERROR("Failed to get json object as '{}'", typeid(D).name());

	return {};
}

#define PA_JSON_SERIALIZE_ENUM(ENUM_TYPE, ...)                                                              \
	template<typename OutputStream = rapidjson::StringBuffer>                                               \
	[[maybe_unused]] inline bool ToJson(rapidjson::Writer<OutputStream>& writer, const ENUM_TYPE& e)        \
	{                                                                                                       \
		static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!");                           \
		static const std::pair<ENUM_TYPE, std::string_view> m[] = __VA_ARGS__;                              \
		auto it = std::ranges::find_if(m,                                                                   \
									   [e](const std::pair<ENUM_TYPE, std::string_view>& ej_pair) -> bool   \
									   {                                                                    \
										   return ej_pair.first == e;                                       \
									   });                                                                  \
		return ::PotatoAlert::Core::ToJson(writer, ((it != std::end(m)) ? it : std::begin(m))->second);     \
	}                                                                                                       \
	[[maybe_unused]] inline rapidjson::GenericStringRef<char> ToJson(const ENUM_TYPE& e)                    \
	{                                                                                                       \
		static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!");                           \
		static const std::pair<ENUM_TYPE, std::string_view> m[] = __VA_ARGS__;                              \
		auto it = std::ranges::find_if(m,                                                                   \
									   [e](const std::pair<ENUM_TYPE, std::string_view>& ej_pair) -> bool   \
									   {                                                                    \
										   return ej_pair.first == e;                                       \
									   });                                                                  \
		return ::PotatoAlert::Core::ToRef((it != std::end(m) ? it : std::begin(m))->second);                \
	}                                                                                                       \
	[[maybe_unused]] inline bool FromJson(const rapidjson::Value& j, ENUM_TYPE& e)                          \
	{                                                                                                       \
		static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!");                           \
		static const std::pair<ENUM_TYPE, std::string_view> m[] = __VA_ARGS__;                              \
		if (!j.IsString())                                                                                  \
			return false;                                                                                   \
		std::string_view key = j.GetString();                                                               \
		auto it = std::ranges::find_if(m,                                                                   \
									   [key](const std::pair<ENUM_TYPE, std::string_view>& ej_pair) -> bool \
									   {                                                                    \
										   return ej_pair.second == key;                                    \
									   });                                                                  \
		if (it == std::end(m))                                                                              \
			return false;                                                                                   \
		e = it->first;                                                                                      \
		return true;                                                                                        \
	}

#define PA_JSON_SERIALIZE_ENUM_PAIRS(ENUM_TYPE, PAIRS)                                                          \
	template<typename OutputStream = rapidjson::StringBuffer>                                                   \
	[[maybe_unused]] inline bool ToJson(rapidjson::Writer<OutputStream>& writer, const ENUM_TYPE& e)            \
	{                                                                                                           \
		static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!");                               \
		auto it = std::ranges::find_if(PAIRS,                                                                   \
									   [e](const std::pair<ENUM_TYPE, std::string_view>& ej_pair) -> bool       \
									   {                                                                        \
										   return ej_pair.first == e;                                           \
									   });                                                                      \
		return ::PotatoAlert::Core::ToJson(writer, ((it != std::end(PAIRS)) ? it : std::begin(PAIRS))->second); \
	}                                                                                                           \
	[[maybe_unused]] inline bool FromJson(const rapidjson::Value& j, ENUM_TYPE& e)                              \
	{                                                                                                           \
		static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!");                               \
		if (!j.IsString())                                                                                      \
			return false;                                                                                       \
		std::string_view key = j.GetString();                                                                   \
		auto it = std::ranges::find_if(PAIRS,                                                                   \
									   [key](const std::pair<ENUM_TYPE, std::string_view>& ej_pair) -> bool     \
									   {                                                                        \
										   return ej_pair.second == key;                                        \
									   });                                                                      \
		if (it == std::end(PAIRS))                                                                              \
			return false;                                                                                       \
		e = it->first;                                                                                          \
		return true;                                                                                            \
	}

#define PA_JSON_SERIALIZE_ENUM_MAP(ENUM_TYPE, MAP)                                                          \
	template<typename OutputStream = rapidjson::StringBuffer>                                               \
	[[maybe_unused]] inline bool ToJson(rapidjson::Writer<OutputStream>& writer, const ENUM_TYPE& e)        \
	{                                                                                                       \
		static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!");                           \
		if ((MAP).contains(e))                                                                              \
		{                                                                                                   \
			return ::PotatoAlert::Core::ToJson(writer, (MAP).at(e));                                        \
		}                                                                                                   \
		else                                                                                                \
		{                                                                                                   \
			return ::PotatoAlert::Core::ToJson(writer, (MAP).begin()->second);                              \
		}                                                                                                   \
	}                                                                                                       \
	[[maybe_unused]] inline bool FromJson(const rapidjson::Value& j, ENUM_TYPE& e)                          \
	{                                                                                                       \
		static_assert(std::is_enum_v<ENUM_TYPE>, #ENUM_TYPE " must be an enum!");                           \
		if (!j.IsString())                                                                                  \
			return false;                                                                                   \
		std::string_view key = j.GetString();                                                               \
		auto it = std::ranges::find_if(MAP,                                                                 \
									   [key](const std::pair<ENUM_TYPE, std::string_view>& ej_pair) -> bool \
									   {                                                                    \
										   return ej_pair.second == key;                                    \
									   });                                                                  \
		if (it == std::end(MAP))                                                                            \
			return false;                                                                                   \
		e = it->first;                                                                                      \
		return true;                                                                                        \
	}

}  // namespace PotatoAlert::Core

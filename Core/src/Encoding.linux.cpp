// Copyright 2023 <github.com/razaqq>

#include "Core/Encoding.hpp"
#include "Core/Result.hpp"

#include <cwchar>
#include <filesystem>
#include <string>
#include <span>
#include <system_error>


using PotatoAlert::Core::Result;

namespace {

static inline std::error_code GetError()
{
	return { errno, std::system_category() };
}

}

Result<size_t> PotatoAlert::Core::Utf8ToWide(std::string_view string, std::span<wchar_t> buffer)
{
	if (string.empty())
		return 0;

	std::mbstate_t state = std::mbstate_t();
	const char* p = string.data();
	size_t size = std::mbsrtowcs(buffer.data(), &p, buffer.size(), &state);

	if (size == static_cast<std::size_t>(-1))
		return PA_ERROR(GetError());

	return size + 1;
}

Result<size_t> PotatoAlert::Core::Utf8ToWide(std::string_view string)
{
	if (string.empty())
		return 0;

	std::mbstate_t state = std::mbstate_t();
	const char* p = string.data();
	size_t size = std::mbsrtowcs(nullptr, &p, 0, &state);

	if (size == static_cast<std::size_t>(-1))
		return PA_ERROR(GetError());

	return size + 1;
}

Result<size_t> PotatoAlert::Core::WideToUtf8(std::wstring_view string, std::span<char> buffer)
{
	if (string.empty())
		return 0;

	std::mbstate_t state = std::mbstate_t();
	const wchar_t* p = string.data();
	size_t size = std::wcsrtombs(buffer.data(), &p, buffer.size(), &state);

	if (size == static_cast<std::size_t>(-1))
		return PA_ERROR(GetError());

	return size + 1;
}

Result<size_t> PotatoAlert::Core::WideToUtf8(std::wstring_view string)
{
	if (string.empty())
		return 0;

	std::mbstate_t state = std::mbstate_t();
	const wchar_t* p = string.data();
	size_t size = std::wcsrtombs(nullptr, &p, 0, &state);

	if (size == static_cast<std::size_t>(-1))
		return PA_ERROR(GetError());

	return size + 1;
}

Result<std::string> PotatoAlert::Core::NativeToUtf8(std::string_view string)
{
	return std::string(string);
}

Result<std::string> PotatoAlert::Core::PathToUtf8(const std::filesystem::path& path)
{
	return path.string();
}

Result<std::filesystem::path> PotatoAlert::Core::Utf8ToPath(std::string_view string)
{
	return std::filesystem::path(string);
}

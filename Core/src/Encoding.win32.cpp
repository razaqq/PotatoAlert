// Copyright 2023 <github.com/razaqq>

#include "Core/Encoding.hpp"
#include "Core/Result.hpp"

#define WIN32_GDICAPMASKS
#define WIN32_NLS
#include "win32.h"

#include <filesystem>
#include <string>
#include <span>
#include <system_error>


using PotatoAlert::Core::Result;

namespace {

static inline std::error_code GetError()
{
	return { static_cast<int>(GetLastError()), std::system_category() };
}

}

Result<size_t> PotatoAlert::Core::Utf8ToWide(std::string_view string, std::span<wchar_t> buffer)
{
	if (string.empty())
		return 0;

	const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string.data(), static_cast<int>(string.size()), buffer.data(), static_cast<int>(buffer.size()));

	if (size == 0)
		return PA_ERROR(GetError());

	return static_cast<size_t>(size);
}

Result<size_t> PotatoAlert::Core::Utf8ToWide(std::string_view string)
{
	return Utf8ToWide(string, {});
}

Result<size_t> PotatoAlert::Core::WideToUtf8(std::wstring_view string, std::span<char> buffer)
{
	if (string.empty())
		return 0;

	const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, string.data(), static_cast<int>(string.size()), buffer.data(), static_cast<int>(buffer.size()), nullptr, nullptr);

	if (size == 0)
		return PA_ERROR(GetError());

	return static_cast<size_t>(size);
}

Result<size_t> PotatoAlert::Core::WideToUtf8(std::wstring_view string)
{
	return WideToUtf8(string, {});
}

Result<std::string> PotatoAlert::Core::NativeToUtf8(std::string_view string)
{
	if (string.empty())
		return "";

	const int utf16Size = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, string.data(), string.size(), nullptr, 0);
	if (utf16Size == 0)
		return PA_ERROR(GetError());

	std::wstring utf16String(utf16Size, '\0');
	if (MultiByteToWideChar(CP_ACP, MB_COMPOSITE, string.data(), string.size(), utf16String.data(), utf16Size) != utf16Size)
		return PA_ERROR(GetError());

	const int utf8Size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16String.c_str(), utf16String.size(), nullptr, 0, nullptr, nullptr);
	if (utf8Size == 0)
		return PA_ERROR(GetError());

	std::string utf8String(utf8Size, '\0');
	if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16String.c_str(), utf16String.size(), utf8String.data(), utf8Size, nullptr, nullptr) != utf8Size)
	{
		return PA_ERROR(GetError());
	}

	return utf8String;
}

Result<std::string> PotatoAlert::Core::PathToUtf8(const std::filesystem::path& path)
{
	const std::wstring str = path.native();
	PA_TRY(length, WideToUtf8(str));
	std::string out(length, '\0');
	PA_TRYD(WideToUtf8(str, out));
	return out;
}

Result<std::filesystem::path> PotatoAlert::Core::Utf8ToPath(std::string_view string)
{
	PA_TRY(length, Utf8ToWide(string));
	std::wstring out(length, '\0');
	PA_TRYD(Utf8ToWide(string, out));
	return std::filesystem::path(out);
}

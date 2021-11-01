// Copyright 2021 <github.com/razaqq>

#include "File.hpp"

#include "Flags.hpp"
#include "win32.h"

#include <string>
#include <vector>


using PotatoAlert::File;

template<typename T>
static constexpr T CreateHandle(HANDLE handle)
{
	return static_cast<T>(reinterpret_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(File::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

namespace {

	struct FileOpenParams
	{
		DWORD access;
		DWORD share;
	};

	FileOpenParams GetFileOpenParams(File::Flags flags)
	{
		FileOpenParams params{};

		params.access = 0;
		params.share = 0;

		if (HasFlag(flags, File::Flags::Read))
		{
			params.access |= FILE_GENERIC_READ;
		}

		if (HasFlag(flags, File::Flags::Write))
		{
			params.access |= FILE_GENERIC_WRITE;
		}
		else
		{
			params.share |= FILE_SHARE_READ;
		}

		return params;
	}

} // namespace


File::Handle File::RawOpen(std::string_view path, Flags flags)
{
	Flags f = flags & (Flags::Open | Flags::Create);

	DWORD mode;
	if (f == Flags::Open)
		mode = OPEN_EXISTING;
	else if (f == Flags::Create)
		mode = CREATE_NEW;
	else if (f == (Flags::Open | Flags::Create))
		mode = OPEN_ALWAYS;
	else
		return Handle::Null;

	FileOpenParams params = GetFileOpenParams(flags);

	DWORD extraFlags = FILE_ATTRIBUTE_NORMAL;
	if (HasFlag(flags, Flags::NoBuffer))
		extraFlags |= FILE_FLAG_NO_BUFFERING;

	std::string pathString(path);
	HANDLE hFile = CreateFileA(pathString.c_str(), params.access, params.share, nullptr, mode, extraFlags, nullptr);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return Handle::Null;
	}

	return CreateHandle<Handle>(hFile);
}

void File::RawClose(Handle handle)
{
	CloseHandle(UnwrapHandle<HANDLE>(handle));
}

uint64_t File::RawGetSize(Handle handle)
{
	LARGE_INTEGER largeInteger;
	GetFileSizeEx(UnwrapHandle<HANDLE>(handle), &largeInteger);
	return largeInteger.QuadPart;
}

bool File::RawRead(Handle handle, std::vector<std::byte>& out)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	ResetFilePointer(handle);

	DWORD dwBytesRead;
	const uint64_t size = RawGetSize(handle);

	out.resize(size);
	void* buff = std::data(out);

	return ReadFile(UnwrapHandle<HANDLE>(handle), buff, static_cast<DWORD>(size), &dwBytesRead, nullptr);
}

bool File::RawReadString(Handle handle, std::string& out)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	ResetFilePointer(handle);

	DWORD dwBytesRead;
	const uint64_t size = RawGetSize(handle);

	out.resize(size);
	void* buff = std::data(out);

	return ReadFile(UnwrapHandle<HANDLE>(handle), buff, static_cast<DWORD>(size), &dwBytesRead, nullptr);
}

bool File::RawWrite(Handle handle, std::span<const std::byte> data)
{
	if (handle == Handle::Null)
	{
		return false;
	}
	
	ResetFilePointer(handle);

	DWORD dwBytesWritten = 0;
	if (WriteFile(UnwrapHandle<HANDLE>(handle), data.data(), data.size(), &dwBytesWritten, nullptr))
	{
		return dwBytesWritten == data.size();
	}
	return false;
}

bool File::RawWriteString(Handle handle, const std::string& data)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	ResetFilePointer(handle);

	DWORD dwBytesWritten = 0;
	if (WriteFile(UnwrapHandle<HANDLE>(handle), data.c_str(), data.length(), &dwBytesWritten, nullptr))
	{
		return dwBytesWritten == data.length();
	}
	return false;
}

bool File::RawFlushBuffer(Handle handle)
{
	return FlushFileBuffers(UnwrapHandle<HANDLE>(handle));
}


std::string File::LastError()
{
	const DWORD err = GetLastError();
	LPSTR lpMsgBuf;
	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPTSTR>(&lpMsgBuf),
			0, nullptr
	);
	return std::string(lpMsgBuf);
}

bool File::GetVersion(std::string_view fileName, Version& outVersion)
{
	const DWORD size = GetFileVersionInfoSizeA(fileName.data(), nullptr);
	if (size == 0)
		return {};

	std::unique_ptr<char[]> versionInfo(new char[size]);
	if (!GetFileVersionInfoA(fileName.data(), 0, 255, versionInfo.get()))
	{
		return false;
	}

	VS_FIXEDFILEINFO* out;
	UINT outSize = 0;
	if (!VerQueryValueA(&versionInfo[0], "\\", reinterpret_cast<LPVOID*>(&out), &outSize) && outSize > 0)
	{
		return false;
	}

	outVersion = Version(std::format("{}.{}.{}.{}",
									 (out->dwFileVersionMS >> 16) & 0xff,
									 (out->dwFileVersionMS >> 0) & 0xff,
									 (out->dwFileVersionLS >> 16) & 0xff,
									 (out->dwFileVersionLS >> 0) & 0xff));

	return true;
}

bool File::RawMove(std::string_view src, std::string_view dst)
{
	return MoveFileA(src.data(), dst.data());
}

bool File::RawDelete(std::string_view file)
{
	return DeleteFileA(file.data());
}

bool File::RawExists(std::string_view file)
{
	const DWORD dwAttrib = GetFileAttributesA(file.data());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool File::ResetFilePointer(Handle handle)
{
	return SetFilePointer(UnwrapHandle<HANDLE>(handle), 0, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
}

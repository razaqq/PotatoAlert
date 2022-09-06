// Copyright 2021 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/File.hpp"
#include "Core/Flags.hpp"

#include "win32.h"

#include <format>
#include <string>
#include <vector>

using namespace PotatoAlert::Core;

namespace {

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

		if (HasFlag(flags, File::Flags::Append))
		{
			params.access |= FILE_APPEND_DATA;
		}

		if (HasFlag(flags, File::Flags::Truncate))
		{
			params.access |= TRUNCATE_EXISTING;
		}

		return params;
	}

} // namespace


File::Handle File::RawOpen(std::string_view path, Flags flags)
{
	Flags f = flags & (Flags::Open | Flags::Create | Flags::Truncate);

	DWORD mode;
	if (f == Flags::Open)
		mode = OPEN_EXISTING;
	else if (f == Flags::Create)
		mode = CREATE_NEW;
	else if (f == (Flags::Open | Flags::Create))
		mode = OPEN_ALWAYS;
	else if (f == (Flags::Open | Flags::Truncate))
		mode = TRUNCATE_EXISTING;
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

template<is_byte T>
bool File::RawRead(Handle handle, std::vector<T>& out, uint64_t size, bool resetFilePointer)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	if (resetFilePointer)
	{
		RawMoveFilePointer(handle, 0, FilePointerMoveMethod::Begin);
	}

	DWORD dwBytesRead;

	out.resize(size);
	void* buff = std::data(out);

	return ReadFile(UnwrapHandle<HANDLE>(handle), buff, static_cast<DWORD>(size), &dwBytesRead, nullptr);
}
template bool File::RawRead(Handle, std::vector<uint8_t>&, uint64_t, bool);
template bool File::RawRead(Handle, std::vector<int8_t>&, uint64_t, bool);
template bool File::RawRead(Handle, std::vector<std::byte>&, uint64_t, bool);
template bool File::RawRead(Handle, std::vector<unsigned char>&, uint64_t, bool);
template bool File::RawRead(Handle, std::vector<char>&, uint64_t, bool);

template<is_byte T>
bool File::RawReadAll(Handle handle, std::vector<T>& out, bool resetFilePointer)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	if (resetFilePointer)
	{
		RawMoveFilePointer(handle, 0, FilePointerMoveMethod::Begin);
	}

	DWORD dwBytesRead;
	const uint64_t size = RawGetSize(handle);

	out.resize(size);
	void* buff = std::data(out);

	return ReadFile(UnwrapHandle<HANDLE>(handle), buff, static_cast<DWORD>(size), &dwBytesRead, nullptr);
}
template bool File::RawReadAll(Handle, std::vector<uint8_t>&, bool);
template bool File::RawReadAll(Handle, std::vector<int8_t>&, bool);
template bool File::RawReadAll(Handle, std::vector<std::byte>&, bool);
template bool File::RawReadAll(Handle, std::vector<unsigned char>&, bool);
template bool File::RawReadAll(Handle, std::vector<char>&, bool);

bool File::RawReadString(Handle handle, std::string& out, uint64_t size, bool resetFilePointer)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	if (resetFilePointer)
	{
		RawMoveFilePointer(handle, 0, FilePointerMoveMethod::Begin);
	}

	DWORD dwBytesRead;

	out.resize(size);
	void* buff = std::data(out);

	return ReadFile(UnwrapHandle<HANDLE>(handle), buff, static_cast<DWORD>(size), &dwBytesRead, nullptr);
}

bool File::RawReadAllString(Handle handle, std::string& out, bool resetFilePointer)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	if (resetFilePointer)
	{
		RawMoveFilePointer(handle, 0, FilePointerMoveMethod::Begin);
	}

	DWORD dwBytesRead;
	const uint64_t size = RawGetSize(handle);

	out.resize(size);
	void* buff = std::data(out);

	return ReadFile(UnwrapHandle<HANDLE>(handle), buff, static_cast<DWORD>(size), &dwBytesRead, nullptr);
}

template<is_byte T>
bool File::RawWrite(Handle handle, std::span<const T> data, bool resetFilePointer)
{
	if (handle == Handle::Null)
	{
		return false;
	}
	
	if (resetFilePointer)
	{
		RawMoveFilePointer(handle, 0, FilePointerMoveMethod::Begin);
	}

	DWORD dwBytesWritten = 0;
	if (WriteFile(UnwrapHandle<HANDLE>(handle), data.data(), data.size(), &dwBytesWritten, nullptr))
	{
		if (dwBytesWritten != data.size())
		{
			return false;
		}
	}

	if (!SetEndOfFile(UnwrapHandle<HANDLE>(handle)))
	{
		return false;
	}

	return true;
}
template bool File::RawWrite(Handle, std::span<const uint8_t>, bool);
template bool File::RawWrite(Handle, std::span<const int8_t>, bool);
template bool File::RawWrite(Handle, std::span<const std::byte>, bool);
template bool File::RawWrite(Handle, std::span<const unsigned char>, bool);
template bool File::RawWrite(Handle, std::span<const char>, bool);

bool File::RawWriteString(Handle handle, std::string_view data, bool resetFilePointer)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	if (resetFilePointer)
	{
		RawMoveFilePointer(handle, 0, FilePointerMoveMethod::Begin);
	}

	DWORD dwBytesWritten = 0;
	if (WriteFile(UnwrapHandle<HANDLE>(handle), data.data(), data.length(), &dwBytesWritten, nullptr))
	{
		if (dwBytesWritten != data.length())
		{
			return false;
		}
	}

	if (!SetEndOfFile(UnwrapHandle<HANDLE>(handle)))
	{
		return false;
	}

	return true;
}

bool File::RawFlushBuffer(Handle handle)
{
	return FlushFileBuffers(UnwrapHandle<HANDLE>(handle));
}

std::string File::RawLastError()
{
	DWORD err = ::GetLastError();
	if (err == 0)
	{
		return "";
	}

	LPSTR lpMsgBuf;
	DWORD size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&lpMsgBuf),
		0, nullptr);
	
	std::string msg(lpMsgBuf, size);
	LocalFree(lpMsgBuf);

	return msg;
}

bool File::GetVersion(std::string_view fileName, Version& outVersion)
{
	const DWORD size = GetFileVersionInfoSizeA(fileName.data(), nullptr);
	if (size == 0)
		return {};

	const std::unique_ptr<char[]> versionInfo(new char[size]);
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

bool File::RawMoveFilePointer(Handle handle, int64_t offset, FilePointerMoveMethod method)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	DWORD moveMethod = FILE_BEGIN;
	switch (method)
	{
		case FilePointerMoveMethod::Begin:
			moveMethod = FILE_BEGIN;
			break;
		case FilePointerMoveMethod::Current:
			moveMethod = FILE_CURRENT;
			break;
		case FilePointerMoveMethod::End:
			moveMethod = FILE_END;
			break;
	}
	
	return SetFilePointer(UnwrapHandle<HANDLE>(handle), offset, nullptr, moveMethod) != INVALID_SET_FILE_POINTER;
}

int64_t File::RawCurrentFilePointer(Handle handle)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	return SetFilePointer(UnwrapHandle<HANDLE>(handle), 0, nullptr, FILE_CURRENT);
}

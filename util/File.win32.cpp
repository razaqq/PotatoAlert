// Copyright 2021 <github.com/razaqq>

#include "File.hpp"
#include "Flags.hpp"
#include "win32.h"


using PotatoAlert::File;

#define CREATE_HANDLE(type, handle) ((type)((uintptr_t)(handle) + 1))
#define UNWRAP_HANDLE(type, handle) ((type)((uintptr_t)(handle) - 1))

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

	std::string pathString(path);
	HANDLE hFile = CreateFileA(pathString.c_str(), params.access, params.share, nullptr, mode, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return Handle::Null;
	}

	return CREATE_HANDLE(Handle, hFile);
}

void File::RawClose(Handle handle)
{
	CloseHandle(UNWRAP_HANDLE(HANDLE, handle));
}

uint64_t File::RawGetSize(Handle handle)
{
	LARGE_INTEGER largeInteger;
	GetFileSizeEx(UNWRAP_HANDLE(HANDLE, handle), &largeInteger);
	return largeInteger.QuadPart;
}

bool File::RawRead(Handle handle, std::string& out)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	ResetFilePointer(handle);

	DWORD dwBytesRead;
	static const size_t size = 16384;
	static char buff[size];

	if (ReadFile(UNWRAP_HANDLE(HANDLE, handle), buff, RawGetSize(handle), &dwBytesRead, nullptr))
	{
		buff[dwBytesRead] = '\0';  // add null termination
		out = std::string(buff);
		return true;
	}

	return false;
}

bool File::RawWrite(Handle handle, const std::string& data)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	ResetFilePointer(handle);

	DWORD dwBytesWritten = 0;
	if (WriteFile(UNWRAP_HANDLE(HANDLE, handle), data.c_str(), data.length(), &dwBytesWritten, nullptr))
	{
		return dwBytesWritten == data.length();
	}
	return false;
}

std::string File::LastError()
{
	DWORD err = GetLastError();
	LPSTR lpMsgBuf;
	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, nullptr
	);
	return std::string(lpMsgBuf);
}

bool File::GetVersion(std::string_view fileName, std::string& outVersion)
{
	DWORD size = GetFileVersionInfoSize(fileName.data(), nullptr);
	if (size == 0)
		return {};

	char* versionInfo = new char[size];
	if (!GetFileVersionInfo(fileName.data(), 0, 255, versionInfo))
	{
		delete[] versionInfo;
		return false;
	}

	VS_FIXEDFILEINFO* out;
	UINT outSize = 0;
	if (!VerQueryValue(&versionInfo[0], "\\", reinterpret_cast<LPVOID*>(&out), &outSize) && outSize > 0)
	{
		delete[] versionInfo;
		return false;
	}

	outVersion = std::format("{}.{}.{}.{}",
							 ( out->dwFileVersionMS >> 16 ) & 0xff,
							 ( out->dwFileVersionMS >>  0 ) & 0xff,
							 ( out->dwFileVersionLS >> 16 ) & 0xff,
							 ( out->dwFileVersionLS >>  0 ) & 0xff
	);
	delete[] versionInfo;

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
	DWORD dwAttrib = GetFileAttributes(file.data());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool File::ResetFilePointer(Handle handle)
{
	return SetFilePointer(UNWRAP_HANDLE(HANDLE, handle), 0, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
}

// Copyright 2021 <github.com/razaqq>

#include "Core/File.hpp"
#include "Core/Flags.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <vector>


using PotatoAlert::Core::File;

namespace {

template<typename T>
static constexpr T CreateHandle(int handle)
{
	return static_cast<T>(static_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(File::Handle handle)
{
	return static_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

int GetFileOpenFlags(File::Flags flags)
{
	int f = 0;

	if (HasFlag(flags, File::Flags::NoBuffer))
		f |= O_DIRECT;

	if (HasFlag(flags, File::Flags::Create))
		f |= O_CREAT;

	if (HasFlag(flags, File::Flags::Append))
		f |= O_APPEND;

	if (HasFlag(flags, File::Flags::Truncate))
		f |= O_TRUNC;

	if (HasFlag(flags, File::Flags::Read) && HasFlag(flags, File::Flags::Write))
		f |= O_RDWR;
	else if (HasFlag(flags, File::Flags::Read))
		f |= O_RDONLY;
	else if (HasFlag(flags, File::Flags::Write))
		f |= O_WRONLY;

	return f;
}

} // namespace

File::Handle File::RawOpen(std::string_view path, Flags flags)
{
	std::string pathString(path);
	int fd = open(pathString.c_str(), GetFileOpenFlags(flags), 0755);
	if (fd == -1)
	{
		return Handle::Null;
	}

#if 0
	if (!HasFlag(flags, File::Flags::Open))
	{
		RawClose(CreateHandle<Handle>(fd));
		return Handle::Null;
	}
#endif

	return CreateHandle<Handle>(fd);
}

void File::RawClose(Handle handle)
{
	if (close(UnwrapHandle<int>(handle)) == -1)
	{
		// TODO: handle error
	}
}

uint64_t File::RawGetSize(Handle handle)
{
	int fd = UnwrapHandle<int>(handle);
	__off64_t pos = lseek64(fd, 0, SEEK_END);
	lseek64(fd, 0, SEEK_SET);
	if (pos == -1)
	{
		// TODO: handle error
		return 0;
	}
	return static_cast<uint64_t>(pos);
}

template<typename T>
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

	out.resize(size);

	if (read(UnwrapHandle<int>(handle), std::data(out), size) == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}
template bool File::RawRead(Handle, std::vector<int8_t>&, uint64_t, bool);
template bool File::RawRead(Handle, std::vector<std::byte>&, uint64_t, bool);
template bool File::RawRead(Handle, std::vector<unsigned char>&, uint64_t, bool);
template bool File::RawRead(Handle, std::vector<char>&, uint64_t, bool);

template<typename T>
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

	const uint64_t size = RawGetSize(handle);
	out.resize(size);

	if (read(UnwrapHandle<int>(handle), std::data(out), size) == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}
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

	out.resize(size);

	ssize_t r = read(UnwrapHandle<int>(handle), std::data(out), size);
	if (r == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
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

	const uint64_t size = RawGetSize(handle);
	out.resize(size);

	ssize_t r = read(UnwrapHandle<int>(handle), std::data(out), size);
	if (r == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}

bool File::RawWrite(Handle handle, std::span<const std::byte> data, bool resetFilePointer)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	if (resetFilePointer)
	{
		RawMoveFilePointer(handle, 0, FilePointerMoveMethod::Begin);
	}

	ssize_t bytesWritten = write(UnwrapHandle<int>(handle), std::data(data), data.size());
	if (bytesWritten == -1)
	{
		// TODO: handle error
		return false;
	}

	if (bytesWritten != data.size())
	{
		// TODO: handle error
		return false;
	}

	if (ftruncate64(UnwrapHandle<int>(handle), data.size() == -1))
	{
		// TODO: handle error
		return false;
	}

	return true;
}

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

	ssize_t bytesWritten = write(UnwrapHandle<int>(handle), std::data(data), data.size());
	if (bytesWritten == -1)
	{
		// TODO: handle error
		return false;
	}

	if (bytesWritten != data.size())
	{
		// TODO: handle error
		return false;
	}

	if (ftruncate64(UnwrapHandle<int>(handle), static_cast<off64_t>(data.size())) == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}

bool File::RawFlushBuffer(Handle handle)
{
	if (fsync(UnwrapHandle<int>(handle)) == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}

std::string File::LastError()
{
	int err = errno;
	switch (err)
	{
		case EPERM:
			return "Operation not permitted";
		case ENOENT:
			return "No such file or directory";
		case ESRCH:
			return "No such process";
		case EINTR:
			return "Interrupted system call";
		case EIO:
			return "I/O error";
		case ENXIO:
			return "No such device or address";
		case E2BIG:
			return "Argument list too long";
		case ENOEXEC:
			return "Exec format error";
		case EBADF:
			return "Bad file number";
		case ECHILD:
			return "No child processes";
		case EAGAIN:
			return "Try again";
		case ENOMEM:
			return "Out of memory";
		case EACCES:
			return "Permission denied";
		case EFAULT:
			return "Bad address";
		case ENOTBLK:
			return "Block device required";
		case EBUSY:
			return "Device or resource busy";
		case EEXIST:
			return "File exists";
		case EXDEV:
			return "Cross-device link";
		case ENODEV:
			return "No such device";
		case ENOTDIR:
			return "Not a directory";
		case EISDIR:
			return "Is a directory";
		case EINVAL:
			return "Invalid argument";
		case ENFILE:
			return "File table overflow";
		case EMFILE:
			return "Too many open files";
		case ENOTTY:
			return "Not a typewriter";
		case ETXTBSY:
			return "Text file busy";
		case EFBIG:
			return "File too large";
		case ENOSPC:
			return "No space left on device";
		case ESPIPE:
			return "Illegal seek";
		case EROFS:
			return "Read-only file system";
		case EMLINK:
			return "Too many links";
		case EPIPE:
			return "Broken pipe";
		case EDOM:
			return "Math argument out of domain of func";
		case ERANGE:
			return "Math result not representable";
		default:
			return "Unknown";
	}
}

bool File::GetVersion(std::string_view fileName, Version& outVersion)
{
	// TODO:: this might not exist, but you can get the sha1 build id
	return false;
}

bool File::RawMove(std::string_view src, std::string_view dst)
{
	if (rename(src.data(), dst.data()) == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}

bool File::RawDelete(std::string_view file)
{
	if (remove(file.data()) == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}

bool File::RawExists(std::string_view file)
{
	return access(file.data(), F_OK) != -1;
}

bool File::RawMoveFilePointer(Handle handle, int64_t offset, FilePointerMoveMethod method)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	int whence;
	switch (method)
	{
		case FilePointerMoveMethod::Begin:
			whence = SEEK_SET;
			break;
		case FilePointerMoveMethod::Current:
			whence = SEEK_CUR;
			break;
		case FilePointerMoveMethod::End:
			whence = SEEK_END;
			break;
	}

	if (lseek64(UnwrapHandle<int>(handle), offset, whence) == -1)
	{
		// TODO: handle error
		return false;
	}
	return true;
}

int64_t File::RawCurrentFilePointer(Handle handle)
{
	if (handle == Handle::Null)
	{
		return false;
	}

	return lseek64(UnwrapHandle<int>(handle), 0, SEEK_CUR);
}
// Copyright 2021 <github.com/razaqq>

#define ZLIB_CONST

#include "Core/Zlib.hpp"

#include "zlib.h"

#include <cstring>
#include <span>
#include <vector>


std::vector<std::byte> PotatoAlert::Core::Zlib::Inflate(std::span<const std::byte> in)
{
	std::vector<std::byte> out;
	unsigned char chunk[2048];

	z_stream stream = {};
	int ret = inflateInit(&stream);

	stream.next_in = reinterpret_cast<const Bytef*>(in.data());
	stream.avail_in = static_cast<uInt>(in.size());

	do {
		do {
			stream.next_out = chunk;
			stream.avail_out = sizeof(chunk);

			ret = inflate(&stream, Z_NO_FLUSH);
			switch (ret)
			{
				case Z_OK:
				case Z_STREAM_END:
					break;

				default:
					out.resize(0);
					goto exit;
			}

			size_t size = sizeof(chunk) - stream.avail_out;

			size_t pos = out.size();
			out.resize(out.size() + size);
			std::memcpy(out.data() + pos, chunk, size);
		} while (stream.avail_out == 0);
	} while (ret != Z_STREAM_END);

exit:
	inflateEnd(&stream);
	return out;
}

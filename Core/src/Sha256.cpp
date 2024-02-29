// Copyright 2022 <github.com/razaqq>

#include "Core/Preprocessor.hpp"
#include "Core/Sha256.hpp"

PA_SUPPRESS_WARN_BEGIN
#include <openssl/evp.h>
PA_SUPPRESS_WARN_END

#include <string>


namespace p = PotatoAlert::Core;

namespace {

void ToHex(unsigned char* src, size_t size, std::string& out)
{
	out.resize(size * 2);

	for (size_t i = 0; i < size; i++)
	{
		std::snprintf(out.data() + 2 * i, out.size(), "%.2x", src[i]);
	}
}

}  // namespace

bool p::Sha256(const void* data, size_t size, std::string& hash)
{
	const EVP_MD* sha256 = EVP_sha256();

	unsigned char md[EVP_MAX_MD_SIZE];
	unsigned hashSize;
	if (EVP_Digest(data, size, md, &hashSize, sha256, nullptr))
	{
		ToHex(md, hashSize, hash);
		return true;
	}
	return false;
}

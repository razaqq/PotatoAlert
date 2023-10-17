#include "Core/Bytes.hpp"
#include "Core/File.hpp"
#include "Core/String.hpp"
#include "GameFileUnpack/MoFile.hpp"
#include <stdio.h>


using PotatoAlert::Core::Byte;
using PotatoAlert::Core::File;
using PotatoAlert::Core::String::EndsWith;
using PotatoAlert::Core::String::ToLower;
using PotatoAlert::GameFileUnpack::ParseMachineObjectFile;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Arg 1 needs to be a .mo file");
		return 1;
	}

	const std::string filePath = ToLower(argv[1]);
	if (!EndsWith(filePath, ".mo"))
	{
		fprintf(stderr, "Arg 1 needs to be a .mo file");
		return 1;
	}

	if (File file = File::Open(std::filesystem::path(filePath), File::Flags::Open | File::Flags::Read))
	{
		std::vector<Byte> data;
		if (!file.ReadAll(data))
		{
			fprintf(stderr, "Failed to open file for reading: %s", File::LastError().c_str());
			return 1;
		}

		auto result = ParseMachineObjectFile(data);
		if (!result)
		{
			fprintf(stderr, "Failed to parse .mo file: %d", result.error());
			return 1;
		}
		
		for (const auto& [original, translation] : result.value())
		{
			fprintf(stderr, "%s - %s\n", original.data(), translation.data());
		}
	}
	else
	{
		fprintf(stderr, "Failed to open file for reading: %s", File::LastError().c_str());
		return 1;
	}

	return 0;
}

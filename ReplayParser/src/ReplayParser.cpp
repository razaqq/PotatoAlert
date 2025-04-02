// Copyright 2021 <github.com/razaqq>

#include "Core/Blowfish.hpp"
#include "Core/Bytes.hpp"
#include "Core/Defer.hpp"
#include "Core/File.hpp"
#include "Core/FileMagic.hpp"
#include "Core/FileMapping.hpp"
#include "Core/Instrumentor.hpp"
#include "Core/Json.hpp"
#include "Core/Zlib.hpp"

#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/PacketParser.hpp"
#include "ReplayParser/Packets.hpp"
#include "ReplayParser/ReplayParser.hpp"
#include "ReplayParser/Result.hpp"

#include <filesystem>
#include <ranges>
#include <span>
#include <string>
#include <vector>


namespace fs = std::filesystem;
using namespace PotatoAlert::Core;
using namespace PotatoAlert::ReplayParser;
using PotatoAlert::ReplayParser::ReplayResult;

ReplayResult<Replay> Replay::FromFile(const fs::path& filePath)
{
	PA_PROFILE_FUNCTION();

	File file = File::Open(filePath, File::Flags::Open | File::Flags::Read | File::Flags::ShareRead | File::Flags::ShareWrite);
	if (!file)
	{
		return PA_REPLAY_ERROR("Failed to open replay file: {}", file.LastError());
	}

	const uint64_t fileSize = file.Size();

	FileMapping fileMapping = FileMapping::Open(file, FileMapping::Flags::Read, fileSize);
	if (!fileMapping)
	{
		return PA_REPLAY_ERROR("Failed to map replay file: {}", fileMapping.LastError());
	}
	PA_DEFER
	{
		fileMapping.Close();
	};

	void* dataPtr = fileMapping.Map(FileMapping::Flags::Read, 0, fileSize);
	if (!dataPtr)
	{
		return PA_REPLAY_ERROR("Failed to map replay file: {}", fileMapping.LastError());
	}
	PA_DEFER
	{
		fileMapping.Unmap(dataPtr, fileSize);
	};
	std::span<const Byte> data{ static_cast<const Byte*>(dataPtr), fileSize };

	Replay replay;

	if (data.size() < 8)
	{
		return PA_REPLAY_ERROR("Replay has invalid length {} < 8.", data.size());
	}

	if (!Core::FileMagic<'\x12', '2', '4', '\x11'>(data))
	{
		return PA_REPLAY_ERROR("Replay has invalid file signature.");
	}

	uint32_t blocksCount;
	if (!TakeInto(data, blocksCount))
	{
		return PA_REPLAY_ERROR("Replay is missing blocksCount.");
	}

	uint32_t metaSize;
	if (!TakeInto(data, metaSize))
	{
		return PA_REPLAY_ERROR("Replay is missing metaSize.");
	}

	if (data.size() < metaSize)
	{
		return PA_REPLAY_ERROR("Replay is missing meta info.");
	}
	replay.MetaString.resize(metaSize);
	std::memcpy(replay.MetaString.data(), Take(data, metaSize).data(), metaSize);

	PA_TRYA(replay.Meta, ParseMeta(replay.MetaString));

	for (size_t i = 0; i < blocksCount - 1; i++)
	{
		uint32_t blockSize;
		if (!TakeInto(data, blockSize))
		{
			return PA_REPLAY_ERROR("Replay is missing blockSize.");
		}
		if (blockSize > 0)
			Take(data, blockSize);
	}
	
	uint32_t decompressedSize;
	if (!TakeInto(data, decompressedSize))
	{
		return PA_REPLAY_ERROR("Replay is missing decompressedSize.");
	}

	uint32_t streamSize;
	if (!TakeInto(data, streamSize))
	{
		return PA_REPLAY_ERROR("Replay is missing streamSize.");
	}

	if (data.size() != streamSize)
	{
		//return PA_REPLAY_ERROR("Replay data != streamSize");
	}

	if (data.size() % Blowfish::BlockSize() != 0)
	{
		return PA_REPLAY_ERROR("Replay data is not a multiple of blowfish block size.");
	}

	std::vector<Byte> decrypted{};
	decrypted.resize(data.size(), Byte{ 0 });

	constexpr std::array<Byte, 16> key = { 0x29, 0xB7, 0xC9, 0x09, 0x38, 0x3F, 0x84, 0x88, 0xFA, 0x98, 0xEC, 0x4E, 0x13, 0x19, 0x79, 0xFB };
	const Blowfish blowfish(key);

	Byte prev[8] = { Byte{ 0 } };
	for (size_t i = 0; i < data.size() / Blowfish::BlockSize(); i++)
	{
		const size_t offset = i * Blowfish::BlockSize();

		uint32_t block[2];
		std::memcpy(block, data.data() + offset, sizeof(block));

		Blowfish::ReverseByteOrder(block[0]);
		Blowfish::ReverseByteOrder(block[1]);
		blowfish.DecryptBlock(&block[0], &block[1]);
		Blowfish::ReverseByteOrder(block[0]);
		Blowfish::ReverseByteOrder(block[1]);

		std::memcpy(decrypted.data() + offset, block, sizeof(block));

		for (size_t j = 0; j < Blowfish::BlockSize(); j++)
		{
			decrypted[offset + j] = decrypted[offset + j] ^ prev[j];
			prev[j] = decrypted[offset + j];
		}
	}

	fileMapping.Close();
	file.Close();

	replay.m_decompressed = Zlib::Inflate(decrypted);
	if (replay.m_decompressed.empty())
	{
		return PA_REPLAY_ERROR("Failed to inflate decrypted replay data with zlib.");
	}

	if (replay.m_decompressed.size() != decompressedSize)
	{
		return PA_REPLAY_ERROR("Replay decompressed data != decompressedSize");
	}

	// free memory of decrypted data
	decrypted.clear();
	decrypted.shrink_to_fit();

	return replay;
}

ReplayResult<std::vector<PacketType>> Replay::ParseAllPackets(const fs::path& scriptsPath) const
{
	PA_TRY(ctx, PrepareContext(scriptsPath, Meta.ClientVersionFromExe));

	std::vector<PacketType> packets;

	const auto parser = MakePacketParser(
		On<BasePlayerCreatePacket>([&packets](BasePlayerCreatePacket&& p) { packets.emplace_back(std::move(p)); }),
		On<CellPlayerCreatePacket>([&packets](CellPlayerCreatePacket&& p) { packets.emplace_back(std::move(p)); }),
		On<EntityCreatePacket>([&packets](EntityCreatePacket&& p) { packets.emplace_back(std::move(p)); }),
		On<EntityControlPacket>([&packets](EntityControlPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<EntityEnterPacket>([&packets](EntityEnterPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<EntityLeavePacket>([&packets](EntityLeavePacket&& p) { packets.emplace_back(std::move(p)); }),
		On<EntityMethodPacket>([&packets](EntityMethodPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<EntityPropertyPacket>([&packets](EntityPropertyPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<PlayerOrientationPacket>([&packets](PlayerOrientationPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<PlayerPositionPacket>([&packets](PlayerPositionPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<NestedPropertyUpdatePacket>([&packets](NestedPropertyUpdatePacket&& p) { packets.emplace_back(std::move(p)); }),
		On<MapPacket>([&packets](MapPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<CameraPacket>([&packets](CameraPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<VersionPacket>([&packets](VersionPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<PlayerEntityPacket>([&packets](PlayerEntityPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<CruiseStatePacket>([&packets](CruiseStatePacket&& p) { packets.emplace_back(std::move(p)); }),
		On<CameraFreeLookPacket>([&packets](CameraFreeLookPacket&& p) { packets.emplace_back(std::move(p)); }),
		On<CameraModePacket>([&packets](CameraModePacket&& p) { packets.emplace_back(std::move(p)); }),
		On<ResultPacket>([&packets](ResultPacket&& p) { packets.emplace_back(std::move(p)); })
	);

	PA_TRYV(ParsePackets(ctx, parser));

	return packets;
}

ReplayResult<ReplayMeta> Replay::ParseMeta(std::string_view str)
{
	PA_TRY_OR_ELSE(js, Core::ParseJson(str),
	{
		return PA_REPLAY_ERROR("Failed to parse replay meta as JSON: {}", error);
	});
	ReplayMeta meta;
	PA_TRYV(FromJson(js, meta));
	return std::move(meta);
}

ReplayResult<std::vector<EntitySpec>> Replay::ParseScripts(const std::filesystem::path& scriptsPath)
{
	return ReplayParser::ParseScripts(scriptsPath);
}

ReplayResult<PacketParseContext> Replay::PrepareContext(const std::filesystem::path& scriptsPath, Version version)
{
	PA_TRY(specs, ReplayParser::ParseScripts(scriptsPath));
	return PacketParseContext
	{
		.Specs = std::move(specs),
		.Entities = {},
		.Version = version,
	};
}

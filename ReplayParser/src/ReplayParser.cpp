// Copyright 2021 <github.com/razaqq>

#include "Core/Blowfish.hpp"
#include "Core/Bytes.hpp"
#include "Core/File.hpp"
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
namespace rp = PotatoAlert::ReplayParser;
using PotatoAlert::ReplayParser::ReplayResult;

template<typename R, typename T>
concept const_contiguous_range_of =
		std::ranges::contiguous_range<R const> &&
		std::same_as<
				std::remove_cvref_t<T>,
				std::ranges::range_value_t<R const>>;

namespace std {

template<equality_comparable T, size_t E,
		 const_contiguous_range_of<T> R>
bool operator==(span<T, E> lhs, R const& rhs)
{
	return ranges::equal(lhs, rhs);
}

template<three_way_comparable T, size_t E,
		 const_contiguous_range_of<T> R>
auto operator<=>(span<T, E> lhs, R const& rhs)
{
	return std::lexicographical_compare_three_way(
			lhs.begin(), lhs.end(),
			ranges::begin(rhs), ranges::end(rhs));
}

}  // namespace std


ReplayResult<Replay> Replay::FromFile(std::string_view filePath, std::string_view gameFilePath)
{
	return FromFile(filePath, gameFilePath);
}

ReplayResult<Replay> Replay::FromFile(const fs::path& filePath, const fs::path& gameFilePath)
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

	void* mapping = fileMapping.Map(FileMapping::Flags::Read, 0, fileSize);
	std::span<const Byte> data{ static_cast<const Byte*>(mapping), fileSize };

	Replay replay;

	if (data.size() < 8)
	{
		return PA_REPLAY_ERROR("Replay has invalid length {} < 8.", data.size());
	}

	constexpr std::array<Byte, 4> sig = { 0x12, 0x32, 0x34, 0x11 };
	if (Take(data, 4) != std::span{ sig })
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

	PA_TRY_OR_ELSE(js, Core::ParseJson(replay.MetaString),
	{
		return PA_REPLAY_ERROR("Failed to parse replay meta as JSON: {}", error);
	});
	FromJson(js, replay.Meta);

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

	const std::vector<Byte> decompressed = Zlib::Inflate(decrypted);
	if (decompressed.empty())
	{
		return PA_REPLAY_ERROR("Failed to inflate decrypted replay data with zlib.");
	}

	if (decompressed.size() != decompressedSize)
	{
		return PA_REPLAY_ERROR("Replay decompressed data != decompressedSize");
	}

	// free memory of decrypted data
	decrypted.clear();
	decrypted.shrink_to_fit();

	replay.Specs = ParseScripts(replay.Meta.ClientVersionFromExe, gameFilePath);

	if (replay.Specs.empty())
	{
		return PA_REPLAY_ERROR("Empty entity specs");
	}

	replay.m_packetParser.Specs = replay.Specs;

	std::span out{ decompressed };
	do {
		PA_TRY(packet, ParsePacket(out, replay.m_packetParser, replay.Meta.ClientVersionFromExe));
		replay.Packets.emplace_back(std::move(packet));
	} while (!out.empty());

	// sort the packets by game time
	// std::ranges::sort(replay.Packets, [](const PacketType& a, const PacketType& b)
	// {
	// 	const Packet& aPacket = std::visit([](const auto& x) -> const Packet& { return x; }, a);
	// 	const Packet& bPacket = std::visit([](const auto& x) -> const Packet& { return x; }, b);
	// 	return aPacket.Clock < bPacket.Clock;
	// });

	return replay;
}

ReplayResult<ReplaySummary> rp::AnalyzeReplay(const fs::path& file, const fs::path& gameFilePath)
{
	PA_TRY(replay, Replay::FromFile(file, gameFilePath));
	PA_TRY(summary, replay.Analyze());
	return summary;
}

bool rp::HasGameScripts(Version gameVersion, const fs::path& gameFilePath)
{
	return !ParseScripts(gameVersion, gameFilePath).empty();
}

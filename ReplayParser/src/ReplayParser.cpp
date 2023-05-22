// Copyright 2021 <github.com/razaqq>

#include "Core/Blowfish.hpp"
#include "Core/Bytes.hpp"
#include "Core/File.hpp"
#include "Core/Instrumentor.hpp"
#include "Core/Json.hpp"
#include "Core/Zlib.hpp"

#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/PacketParser.hpp"
#include "ReplayParser/Packets.hpp"
#include "ReplayParser/ReplayParser.hpp"
#include "ReplayParser/Result.hpp"

#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <vector>


using namespace PotatoAlert::Core;
using namespace PotatoAlert::ReplayParser;
namespace rp = PotatoAlert::ReplayParser;
using PotatoAlert::ReplayParser::ReplayResult;

ReplayResult<Replay> Replay::FromFile(std::string_view fileName)
{
	PA_PROFILE_FUNCTION();

	File file = File::Open(fileName, File::Flags::Open | File::Flags::Read);
	if (!file)
	{
		return PA_REPLAY_ERROR("Failed to open replay file: {}", file.LastError());
	}

	Replay replay;
	if (!file.ReadAll(replay.m_rawData))
	{
		return PA_REPLAY_ERROR("Failed to read replay file: {}", file.LastError());
	}
	file.Close();

	replay.m_data = std::span(replay.m_rawData);

	// remove first 8 bytes, no idea what they are, maybe version?
	if (replay.m_data.size() < 8)
	{
		return PA_REPLAY_ERROR("Replay data missing first 8 bytes.");
	}
	Take(replay.m_data, 8);

	uint32_t metaSize;
	if (!TakeInto(replay.m_data, metaSize))
	{
		return PA_REPLAY_ERROR("Replay is missing metaSize.");
	}

	if (replay.m_data.size() < metaSize)
	{
		return PA_REPLAY_ERROR("Replay is missing meta info.");
	}
	replay.MetaString.resize(metaSize);
	std::memcpy(replay.MetaString.data(), Take(replay.m_data, metaSize).data(), metaSize);

	PA_TRY_OR_ELSE(js, Core::ParseJson(replay.MetaString),
	{
		return PA_REPLAY_ERROR("Failed to parse replay meta as JSON: {}", error);
	});
	FromJson(js, replay.Meta);

	return replay;
}

ReplayResult<void> Replay::ReadPackets(const std::vector<fs::path>& scriptsSearchPaths)
{
	PA_PROFILE_FUNCTION();

	uint32_t uncompressedSize;
	if (!TakeInto(m_data, uncompressedSize))
	{
		return PA_REPLAY_ERROR("Replay is missing uncompressedSize.");
	}

	uint32_t streamSize;
	if (!TakeInto(m_data, streamSize))
	{
		return PA_REPLAY_ERROR("Replay is missing streamSize.");
	}

	std::vector<Byte> decrypted{};
	decrypted.resize(m_data.size(), Byte{ 0 });

	if (m_data.size() % Blowfish::BlockSize() != 0)
	{
		return PA_REPLAY_ERROR("Replay data is not a multiple of blowfish block size.");
	}

	constexpr std::array<Byte, 16> key = { 0x29, 0xB7, 0xC9, 0x09, 0x38, 0x3F, 0x84, 0x88, 0xFA, 0x98, 0xEC, 0x4E, 0x13, 0x19, 0x79, 0xFB };
	const Blowfish blowfish(key);

	Byte prev[8] = { Byte{ 0 } };
	for (size_t i = 0; i < m_data.size() / Blowfish::BlockSize(); i++)
	{
		const size_t offset = i * Blowfish::BlockSize();

		uint32_t block[2];
		std::memcpy(block, m_data.data() + offset, sizeof(block));

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

	// free memory of raw data
	m_rawData.clear();
	m_rawData.shrink_to_fit();
	m_data = {};

	const std::vector<Byte> decompressed = Zlib::Inflate(decrypted);
	if (decompressed.empty())
	{
		return PA_REPLAY_ERROR("Failed to inflate decrypted replay data with zlib.");
	}

	// free memory of decrypted data
	decrypted.clear();
	decrypted.shrink_to_fit();

	Specs = ParseScripts(Meta.ClientVersionFromExe, scriptsSearchPaths);

	if (Specs.empty())
	{
		return PA_REPLAY_ERROR("Empty entity specs");
	}

	m_packetParser.Specs = Specs;

	std::span out{ decompressed };
	do {
		PA_TRY(packet, ParsePacket(out, m_packetParser));
		Packets.emplace_back(std::move(packet));
	} while (!out.empty());

	// sort the packets by game time
	std::ranges::sort(Packets, [](const PacketType& a, const PacketType& b)
	{
		const Packet& aPacket = std::visit([](const auto& x) -> const Packet& { return x; }, a);
		const Packet& bPacket = std::visit([](const auto& x) -> const Packet& { return x; }, b);
		return aPacket.Clock < bPacket.Clock;
	});

	return {};
}

ReplayResult<ReplaySummary> rp::AnalyzeReplay(std::string_view file, const std::vector<fs::path>& scriptsSearchPaths)
{
	PA_TRY(replay, Replay::FromFile(file));
	PA_TRYV(replay.ReadPackets(scriptsSearchPaths));
	PA_TRY(summary, replay.Analyze());
	return summary;
}

bool rp::HasGameScripts(const Version& gameVersion, const std::vector<fs::path>& scriptsSearchPaths)
{
	return !ParseScripts(gameVersion, scriptsSearchPaths).empty();
}

// Copyright 2021 <github.com/razaqq>

#include "ReplayParser.hpp"

#include "Core/Blowfish.hpp"
#include "Core/Bytes.hpp"
#include "Core/File.hpp"
#include "Core/Instrumentor.hpp"
#include "Core/Json.hpp"
#include "Core/Zlib.hpp"
#include "GameFiles.hpp"
#include "PacketParser.hpp"
#include "Packets.hpp"
#include "Zlib.hpp"

#include <optional>
#include <span>
#include <string>
#include <vector>


using namespace PotatoAlert::ReplayParser;

std::optional<Replay> Replay::FromFile(std::string_view fileName)
{
	PA_PROFILE_FUNCTION();

	File file = File::Open(fileName, File::Flags::Open | File::Flags::Read);
	if (!file)
	{
		LOG_ERROR("Failed to open replay file.");
		return {};
	}

	Replay replay;
	if (!file.Read(replay.m_rawData))
	{
		LOG_ERROR("Failed to read replay file.");
		return {};
	}
	file.Close();

	replay.m_data = std::span(replay.m_rawData);

	// remove first 8 bytes, no idea what they are, maybe version?
	if (replay.m_data.size() < 8)
	{
		LOG_ERROR("Replay data missing first 8 bytes.");
		return {};
	}
	Take(replay.m_data, 8);

	uint32_t metaSize;
	if (!TakeInto(replay.m_data, metaSize))
	{
		LOG_ERROR("Replay is missing metaSize.");
		return {};
	}

	std::string metaStr;
	if (replay.m_data.size() < metaSize)
	{
		LOG_ERROR("Replay is missing meta info.");
		return {};
	}
	metaStr.resize(metaSize);
	std::memcpy(metaStr.data(), Take(replay.m_data, metaSize).data(), metaSize);

	json js;
	sax_no_exception sax(js);
	if (!json::sax_parse(metaStr, &sax))
	{
		LOG_ERROR("Failed to parse replay meta as JSON.");
		return {};
	}
	replay.meta = js.get<ReplayMeta>();

	return replay;
}

bool Replay::ReadPackets()
{
	PA_PROFILE_FUNCTION();

	uint32_t uncompressedSize;
	if (!TakeInto(m_data, uncompressedSize))
	{
		LOG_ERROR("Replay is missing uncompressedSize.");
		return false;
	}

	uint32_t streamSize;
	if (!TakeInto(m_data, streamSize))
	{
		LOG_ERROR("Replay is missing streamSize.");
		return false;
	}

	std::vector<std::byte> decrypted{};
	decrypted.resize(m_data.size(), std::byte{ 0 });

	if (m_data.size() % Blowfish::BlockSize() != 0)
	{
		LOG_ERROR("Replay data is not a multiple of blowfish block size.");
		return false;
	}

	std::array<std::byte, 16> key = MakeBytes<std::byte>(0x29, 0xB7, 0xC9, 0x09, 0x38, 0x3F, 0x84, 0x88, 0xFA, 0x98, 0xEC, 0x4E, 0x13, 0x19, 0x79, 0xFB);
	const Blowfish blowfish(std::span<std::byte>{ key });

	std::byte prev[8] = { std::byte{ 0 } };
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

	auto decompressed = Zlib::Inflate(decrypted);
	if (decompressed.empty())
	{
		LOG_ERROR("Failed to inflate decrypted replay data with zlib.");
		return false;
	}

	specs = ParseScripts(meta.clientVersionFromExe);
	PacketParser parser{ specs, {} };

	std::span out{ decompressed };
	do {
		packets.emplace_back(ParsePacket(out, parser));
	} while (!out.empty());
	
	m_rawData.clear();

	return true;
}

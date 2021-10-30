// Copyright 2021 <github.com/razaqq>

#include "ReplayParser.hpp"

#include "Blowfish.hpp"
#include "ByteUtil.hpp"
#include "GameFiles.hpp"
#include "Json.hpp"
#include "PacketParser.hpp"
#include "Packets.hpp"
#include "Version.hpp"
#include "Zlib.hpp"

#include <optional>
#include <span>
#include <string>
#include <vector>


using namespace PotatoAlert::ReplayParser;

/*
static std::unordered_map<std::string, uint32_t> g_battleEndMethod =
{
	{ "0.9.9", 17 },
	{ "0.9.10", 17 },
	{ "0.9.11", 17 },
	{ "0.9.12", 17 },
	{ "0.10.0", 17 },
	{ "0.10.1", 17 },
	{ "0.10.2", 17 },
	{ "0.10.3", 17 },
	{ "0.10.4", 17 },
	{ "0.10.5", 17 },
	{ "0.10.6", 17 },
	{ "0.10.7", 19 },
	{ "0.10.8", 19 },
};
*/

template<typename R, typename T>
static R VariantCast(T&& t)
{
	return std::visit(
		[]<typename T0>(T0&& val) -> R
		{
			return { std::forward<T0>(val) };
		}, std::forward<T>(t));
}

Packet ParsePacket(std::span<std::byte>& data, PacketParser& parser)
{
	uint32_t size;
	if (!TakeInto(data, size))
		return {};
	uint32_t type;
	if (!TakeInto(data, type))
		return {};
	float clock;
	if (!TakeInto(data, clock))
		return {};

	auto raw = Take(data, size);

	switch (type)
	{
		// TODO: extra fields are thrown away
		case static_cast<uint32_t>(PacketBaseType::BasePlayerCreate):
			return VariantCast<Packet>(ParseBasePlayerCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::CellPlayerCreate):
			return VariantCast<Packet>(ParseCellPlayerCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityControl):
			return VariantCast<Packet>(ParseEntityControlPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityEnter):
			return VariantCast<Packet>(ParseEntityEnterPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityLeave):
			return VariantCast<Packet>(ParseEntityLeavePacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityCreate):
			return VariantCast<Packet>(ParseEntityCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityProperty):
			return VariantCast<Packet>(ParseEntityPropertyPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityMethod):
			return VariantCast<Packet>(ParseEntityMethodPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerPosition):
			return VariantCast<Packet>(ParsePlayerPositionPacketPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::NestedPropertyUpdate):
			return VariantCast<Packet>(ParseNestedPropertyUpdatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerOrientation):
			return VariantCast<Packet>(ParsePlayerOrientationPacket(raw, clock));
		default:  // unknown
			break;
	}

	return UnknownPacket{};
}

std::optional<ReplayFile> ReplayFile::FromFile(std::string_view fileName)
{
	File file = File::Open(fileName, File::Flags::Open | File::Flags::Read);
	if (!file)
	{
		LOG_ERROR("Failed to open replay file.");
		return {};
	}

	std::vector<std::byte> data;
	if (!file.Read(data))
	{
		LOG_ERROR("Failed to read replay file.");
		return {};
	}
	file.Close();

	std::span remaining(data);

	// remove first 8 bytes, no idea what they are, maybe version?
	if (remaining.size() < 8)
	{
		LOG_ERROR("Replay data missing first 8 bytes.");
		return {};
	}
	Take(remaining, 8);

	uint32_t metaSize;
	if (!TakeInto(remaining, metaSize))
	{
		LOG_ERROR("Replay is missing metaSize.");
		return {};
	}

	std::string metaStr;
	if (remaining.size() < metaSize)
	{
		LOG_ERROR("Replay is missing meta info.");
		return {};
	}
	metaStr.resize(metaSize);
	std::memcpy(metaStr.data(), Take(remaining, metaSize).data(), metaSize);

	json js;
	sax_no_exception sax(js);
	if (!json::sax_parse(metaStr, &sax))
	{
		LOG_ERROR("Failed to parse replay meta as JSON.");
		return {};
	}
	const auto replayMeta = js.get<ReplayMeta>();

	uint32_t uncompressedSize;
	if (!TakeInto(remaining, uncompressedSize))
	{
		LOG_ERROR("Replay is missing uncompressedSize.");
		return {};
	}

	uint32_t streamSize;
	if (!TakeInto(remaining, streamSize))
	{
		LOG_ERROR("Replay is missing streamSize.");
		return {};
	}

	std::vector<std::byte> decrypted{};
	decrypted.resize(remaining.size(), std::byte{ 0 });

	if (remaining.size() % Blowfish::BlockSize() != 0)
	{
		LOG_ERROR("Replay data is not a multiple of blowfish block size.");
		return {};
	}

	std::array<std::byte, 16> key = MakeBytes(0x29, 0xB7, 0xC9, 0x09, 0x38, 0x3F, 0x84, 0x88, 0xFA, 0x98, 0xEC, 0x4E, 0x13, 0x19, 0x79, 0xFB);
	const Blowfish blowfish(std::span<std::byte>{ key });

	std::byte prev[8] = { std::byte{ 0 } };
	for (size_t i = 0; i < remaining.size() / Blowfish::BlockSize(); i++)
	{
		const size_t offset = i * Blowfish::BlockSize();

		uint32_t block[2];
		std::memcpy(block, remaining.data() + offset, sizeof(block));

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
		return {};
	}

	PacketParser parser{ ParseScripts(Version(replayMeta.clientVersionFromExe)) };

	std::span out{ decompressed };
	std::vector<Packet> packets;
	do {
		packets.emplace_back(ParsePacket(out, parser));
	} while (!out.empty());
	
	return ReplayFile{ replayMeta, packets };
}

std::optional<bool> ReplayFile::Won() const
{
	return false;
	/*
	Version v;
	if (!(v = Version(this->meta.clientVersionFromExe)))
	{
		return {};
	}
	const std::string versionString = v.ToString();
	if (!g_battleEndMethod.contains(versionString))
	{
		return {};
	}
	const size_t battleEndMethod = g_battleEndMethod[versionString];
	
	for (auto& packet : packets)
	{
		if (std::holds_alternative<EntityMethodPacket>(packet))
		{
			EntityMethodPacket p = std::get<EntityMethodPacket>(packet);
			if (p.methodId == battleEndMethod)
			{
				std::span raw{ p.data };
				int8_t winningTeam;
				std::memcpy(&winningTeam, Take(raw, sizeof(winningTeam)).data(), sizeof(winningTeam));
				if (winningTeam != 0 && winningTeam != 1)
				{
					return {};
				}
				return winningTeam == 0;
			}
		}
	}
	return {};
	*/
}

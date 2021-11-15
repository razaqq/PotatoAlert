// Copyright 2021 <github.com/razaqq>

#include "ReplayParser.hpp"

#include "Blowfish.hpp"
#include "Bytes.hpp"
#include "GameFiles.hpp"
#include "Instrumentor.hpp"
#include "Json.hpp"
#include "PacketParser.hpp"
#include "Packets.hpp"
#include "Zlib.hpp"

#include <optional>
#include <span>
#include <string>
#include <vector>


using namespace PotatoAlert::ReplayParser;

template<typename R, typename T>
static R VariantCast(T&& t)
{
	return std::visit([]<typename T0>(T0&& val) -> R
	{
		return { std::forward<T0>(val) };
	}, std::forward<T>(t));
}

static PacketType ParsePacket(std::span<std::byte>& data, PacketParser& parser)
{
	uint32_t size;
	if (!TakeInto(data, size))
		return InvalidPacket{};
	uint32_t type;
	if (!TakeInto(data, type))
		return InvalidPacket{};
	float clock;
	if (!TakeInto(data, clock))
		return InvalidPacket{};

	auto raw = Take(data, size);

#ifndef NDEBUG
	size_t rawSize = raw.size();
#endif

	switch (type)
	{
#ifndef NDEBUG
		case 0x10:
		{
			bool b1;
			TakeInto(raw, b1);
			LOG_TRACE("0x10: {} -> {} bytes", b1, rawSize);
			break;
		}
		case 0xE:
		{
			uint32_t f1, f2;
			// 2454267026 1069697316
			TakeInto(raw, f1);
			TakeInto(raw, f2);
			// LOG_TRACE("0xE: {} {}", f1, f2);
			break;
		}
		case 0x18:
		{
			// LOG_TRACE("0x18: {}", FormatBytes(raw));
			break;
		}
		case 0x2E:
		{
			bool b2;
			TakeInto(raw, b2);
			LOG_TRACE("0x2E: {} -> {} bytes", b2, rawSize);
			break;
		}
		case 0xF:
		{
			LOG_TRACE("0xF: {} bytes", raw.size());
			break;
		}
		case 0x26:
		{
			uint32_t unknown;
			TakeInto(raw, unknown);
			LOG_TRACE("0x26: {} -> {} bytes", unknown, rawSize);
			break;
		}
		case 0x20:
		{
			uint32_t unknown;
			TakeInto(raw, unknown);
			LOG_TRACE("0x20: {} -> {} bytes", unknown, rawSize);
			break;
		}
		case 0x29:
		{
			// 32 bytes
			LOG_TRACE("0x29: {} bytes", rawSize);
			break;
		}
		case 0x2F:
		{
			Vec3 v;
			TakeInto(raw, v);
			LOG_TRACE("0x2F: {{{}, {}, {}}} -> {} bytes", v.x, v.y, v.z, rawSize);
			break;
		}
		case 0xFFFFFFFF:
		{
			LOG_TRACE("0xFFFFFFFF: {} bytes", rawSize);
			break;
		}
#endif
		case static_cast<uint32_t>(PacketBaseType::Version):
			return VariantCast<PacketType>(ParseVersionPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::BasePlayerCreate):
			return VariantCast<PacketType>(ParseBasePlayerCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::CellPlayerCreate):
			return VariantCast<PacketType>(ParseCellPlayerCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityControl):
			return VariantCast<PacketType>(ParseEntityControlPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityEnter):
			return VariantCast<PacketType>(ParseEntityEnterPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityLeave):
			return VariantCast<PacketType>(ParseEntityLeavePacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityCreate):
			return VariantCast<PacketType>(ParseEntityCreatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityProperty):
			return VariantCast<PacketType>(ParseEntityPropertyPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::EntityMethod):
			return VariantCast<PacketType>(ParseEntityMethodPacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerPosition):
			return VariantCast<PacketType>(ParsePlayerPositionPacketPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::NestedPropertyUpdate):
			return VariantCast<PacketType>(ParseNestedPropertyUpdatePacket(raw, parser, clock));
		case static_cast<uint32_t>(PacketBaseType::PlayerOrientation):
			return VariantCast<PacketType>(ParsePlayerOrientationPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::Camera):
			return VariantCast<PacketType>(ParseCameraPacket(raw, clock));
		case static_cast<uint32_t>(PacketBaseType::Map):
			return VariantCast<PacketType>(ParseMapPacket(raw, clock));
		default:  // unknown
			break;
	}

	return UnknownPacket{};
}

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

	PacketParser parser{ ParseScripts(meta.clientVersionFromExe), {} };

	std::span out{ decompressed };
	do {
		packets.emplace_back(ParsePacket(out, parser));
	} while (!out.empty());
	
	m_rawData.clear();

	return true;
}

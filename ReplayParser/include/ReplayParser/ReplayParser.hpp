// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Preprocessor.hpp"

#include "ReplayParser/Packets.hpp"
#include "ReplayParser/PacketParser.hpp"
#include "ReplayParser/ReplayMeta.hpp"
#include "ReplayParser/ReplaySummary.hpp"
#include "ReplayParser/Result.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>


namespace PotatoAlert::ReplayParser {

enum class DamageFlag : int64_t
{
	EnemyDamage = 0,
	AllyDamage = 1,
	SpottingDamage = 2,
	PotentialDamage = 3,
};

#define DAMAGE_TYPES            \
	X(MainAP, 1)                \
	X(MainHE, 2)                \
	X(SecondaryAP, 3)           \
	X(SecondaryHE, 4)           \
	X(ShipTorpedo, 7)           \
	X(AviaBombAP, 10)           \
	X(AviaBombHE, 11)           \
	X(AviaTorpedo, 12)          \
	X(Fire, 17)                 \
	X(Ram, 18)                  \
	X(Flooding, 20)             \
	X(AviaBomb, 27)             \
	X(AviaRocket, 28)           \
	X(MainCS, 32)               \
	X(SecondarySAP, 33)         \
	X(Unknown, 41)              \
	X(AviaSkipBombHE, 42)       \
	X(AviaBombAirstrike, 52)    \
	X(DepthChargeAirstrike, 58) \
	X(AcousticHomingTorpedo, 60)

enum class DamageType : int64_t
{
#define X(name, value) name = (value),
	DAMAGE_TYPES
#undef X
};

[[clang::no_destroy]] static const std::unordered_map<DamageType, std::string_view> g_damageTypes =
{
#define X(name, value) { DamageType::name, #name },
	DAMAGE_TYPES
#undef X
};

inline std::string_view GetName(DamageType dmgType)
{
	if (g_damageTypes.contains(dmgType))
		return g_damageTypes.at(dmgType);
	return "Unknown";
}

constexpr inline std::string_view TierToString(uint8_t tier)
{
	switch (tier)
	{
		case 1:
			return "I";
		case 2:
			return "II";
		case 3:
			return "III";
		case 4:
			return "IV";
		case 5:
			return "V";
		case 6:
			return "VI";
		case 7:
			return "VII";
		case 8:
			return "VIII";
		case 9:
			return "IX";
		case 10:
			return "X";
		default:
			return "Err";
	}
}

class PA_API Replay
{
public:
	std::string MetaString;
	ReplayMeta Meta;
	//std::vector<PacketType> Packets;

	static ReplayResult<Replay> FromFile(const std::filesystem::path& filePath);
	ReplayResult<std::vector<PacketType>> ParseAllPackets(const std::filesystem::path& scriptsPath) const;

	ReplayResult<void> ParsePackets(PacketParseContext& ctx, auto&& parser) const
	{
		Core::ByteReader<> reader(m_decompressed);
		PA_TRYV(ReplayParser::ParsePackets(reader, ctx, parser));
		return {};
	}

	static ReplayResult<ReplayMeta> ParseMeta(std::string_view str);
	static ReplayResult<std::vector<EntitySpec>> ParseScripts(const std::filesystem::path& scriptsPath);
	static ReplayResult<PacketParseContext> PrepareContext(const std::filesystem::path& scriptsPath, Core::Version version);

	[[nodiscard]] ReplayResult<ReplaySummary> Analyze(const std::filesystem::path& scriptsPath) const;

	void Free()
	{
		// free memory of decompressed data
		m_decompressed.clear();
		m_decompressed.shrink_to_fit();
	}

private:
	std::vector<Byte> m_decompressed;
};

}  // namespace PotatoAlert::ReplayParser

// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/Packets.hpp"
#include "ReplayParser/PacketParser.hpp"
#include "ReplayParser/ReplayMeta.hpp"
#include "ReplayParser/Result.hpp"

#include <filesystem>
#include <span>
#include <string>
#include <vector>


namespace PotatoAlert::ReplayParser {

#define DAMAGE_TYPES            \
	X(MainAP,                1) \
	X(MainHE,                2) \
	X(SecondaryAP,           3) \
	X(SecondaryHE,           4) \
	X(ShipTorpedo,           7) \
	X(AviaBombAP,            10) \
	X(AviaBombHE,            11) \
	X(AviaTorpedo,           12) \
	X(Fire,                  17) \
	X(Ram,                   18) \
	X(Flooding,              20) \
	X(AviaBomb,              27) \
	X(AviaRocket,            28) \
	X(MainCS,                32) \
	X(SecondarySAP,          33) \
	X(Unknown,               41) \
	X(AviaSkipBombHE,        42) \
	X(AviaBombAirstrike,     52) \
	X(DepthChargeAirstrike,  58) \
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

#define RIBBON_TYPES                                    \
	X(Wave, -99, Wave)                                  \
	X(SonarHit, -98, SonarHit)                          \
	X(PulsePhaser, -97, PulsePhaser)                    \
	X(Shield, -96, Shield)                              \
	X(UnknownRibbon, -1, UnknownRibbon)                 \
	X(Artillery, 0, Artillery)                          \
	X(TorpedoHit, 1, TorpedoHit)                        \
	X(Bomb, 2, Bomb)                                    \
	X(PlaneShotDown, 3, PlaneShotDown)                  \
	X(Incapacitation, 4, Incapacitation)                \
	X(Destroyed, 5, Destroyed)                          \
	X(SetFire, 6, SetFire)                              \
	X(Flooding, 7, Flooding)                            \
	X(Citadel, 8, Citadel)                              \
	X(Defended, 9, Defended)                            \
	X(Captured, 10, Captured)                           \
	X(AssistedInCapture, 11, AssistedInCapture)         \
	X(Suppressed, 12, Suppressed)                       \
	X(SecondaryHit, 13, SecondaryHit)                   \
	X(OverPenetration, 14, Artillery)                   \
	X(Penetration, 15, Artillery)                       \
	X(NonPenetration, 16, Artillery)                    \
	X(Ricochet, 17, Artillery)                          \
	X(BuildingDestroyed, 18, BuildingDestroyed)         \
	X(Spotted, 19, Spotted)                             \
	X(DiveBombOverPenetration, 20, Bomb)                \
	X(DiveBombPenetration, 21, Bomb)                    \
	X(DiveBombNonPenetration, 22, Bomb)                 \
	X(DiveBombRicochet, 23, Bomb)                       \
	X(Rocket, 24, Rocket)                               \
	X(RocketPenetration, 25, Rocket)                    \
	X(RocketNonPenetration, 26, Rocket)                 \
	X(ShotDownByAircraft, 27, ShotDownByAircraft)       \
	X(TorpedoProtectionHit, 28, Artillery)              \
	X(BombBulge, 29, Bomb)                              \
	X(RocketTorpedoProtectionHit, 30, Rocket)           \
	X(DepthChargeHit, 31, DepthChargeHit)               \
	X(AcousticHit, 32, AcousticHit)                     \
	X(BuffSeized, 33, BuffSeized)                       \
	X(RocketRicochet, 34, Rocket)                       \
	X(RocketOverPenetration, 35, Rocket)                \
	X(WaveKillTorpedo, 36, Wave)                        \
	X(WaveCutWave, 37, Wave)                            \
	X(WaveHitVehicle, 38, Wave)                         \
	X(SonarOneHit, 39, SonarHit)                        \
	X(SonarTwoHits, 40, SonarHit)                       \
	X(SonarNeutralized, 41, SonarHit)                   \
	X(Acid, 42, Acid)                                   \
	X(DepthChargeFullDamage, 43, DepthChargeHit)        \
	X(DepthChargePartialDamage, 44, DepthChargeHit)     \
	X(Mine, 45, Mine)                                   \
	X(DeminingMine, 46, DeminingMine)                   \
	X(DeminingMinefield, 47, DeminingMinefield)         \
	X(TorpedoPhotonHit, 48, PulsePhaser)                \
	X(TorpedoPhotonSplash, 49, PulsePhaser)             \
	X(AimPulseTorpedoPhoton, 50, AimPulseTorpedoPhoton) \
	X(PhaserLaser, 51, PhaserLaser)                     \
	X(ShieldHit, 52, Shield)                            \
	X(ShieldRemoved, 53, Shield)


enum class RibbonType : int8_t
{
#define X(name, value, parent) name = (value),
	RIBBON_TYPES
#undef X
};

[[clang::no_destroy]] static const std::unordered_map<RibbonType, std::string_view> g_ribbonNames =
{
#define X(name, value, parent) { RibbonType::name, #name },
	RIBBON_TYPES
#undef X
};
PA_JSON_SERIALIZE_ENUM_MAP(RibbonType, g_ribbonNames);

inline std::string_view GetName(RibbonType ribbon)
{
	if (g_ribbonNames.contains(ribbon))
		return g_ribbonNames.at(ribbon);
	return "Unknown";
}

inline constexpr RibbonType GetParent(RibbonType ribbon)
{
	switch (ribbon)
	{
#define X(name, value, parent)           \
	case RibbonType::name:               \
	{                                    \
		return RibbonType::parent;       \
	}
	RIBBON_TYPES
#undef X
	}
	return RibbonType::UnknownRibbon;
}

#define ACHIEVEMENT_TYPES                 \
	X(UnknownAchievement,              0) \
	X(DevastatingStrike,      4282573744) \
	X(HighCaliber,            4290962352) \
	X(AADefenseExpert,        4111655856) \
	X(ItsJustAFleshWound,     4283622320) \
	X(Dreadnought,            4289913776) \
	X(Confederate,            4288865200) \
	X(KrakenUnleashed,        4269990832) \
	X(FirstBlood,             4277330864) \
	X(CloseQuartersExpert,    4273136560) \
	X(Detonation,             4274185136) \
	X(ShoulderToShoulder,     3909280688) \
	X(Fireproof,              4276282288) \
	X(DieHard,                4279428016) \
	X(SoloWarrior,            4292010928) \
	X(Witherer,               4281525168) \
	X(DoubleStrike,           4293059504) \
	X(Arsionist,              4287816624) \
	X(StrikeTeam,             3912426416) \
	X(GeneralOffensive,       3911377840) \
	X(CoordinatedAttack,      3910329264) \
	X(BrothersInArms,         3908232112) \
	X(ClanBrawlTop1000,       4042449840) \
	X(ClanBrawlTop100,        4043498416) \
	X(ClanBrawlTop10,         4044546992) \
	X(ClanBrawlTop1,          4045595568) \
	X(JollyRoger,             4050838448) \
	X(Unsinkable,             4275233712) \
	X(NaturalSelection,       4166181808) \
	X(Ray,                    4168278960) \
	X(SeaStar,                4192396208) \
	X(UniversalSeaman,        4173521840) \
	X(WeatherBeaten,          4191347632) \
	X(OldTimer,               4190299056) \
	X(ExperiencedOne,         4189250480) \
	X(ImportantMissions,      4187153328) \
	X(SpecialOrders,          4186104752) \
	X(SecretInstructions,     4185056176) \
	X(Exterminator,           4177716144) \
	X(Raider,                 4176667568) \
	X(Ravager,                4175618992) \
	X(Shield,                 4181910448) \
	X(Guardian,               4180861872) \
	X(Protector,              4179813296) \
	X(AnInstantBeforeVictory, 4164084656) \
	X(SaveCommanderJenkins,   4182959024) \
	X(CrashTester,            4165133232) \
	X(SharkAmongShrimps,      4104315824) \
	X(InsertCoin,             4103267248) \
	X(MajorContribution,      4163036080) \
	X(Assistant,              4170376112) \
	X(WillToWin,              4193444784) \
	X(TacticalExpertise,      4161987504) \
	X(CombatScout,            3879920560)
	// X(JollyRogerSilver,    4045595568) \  // TODO
	// X(JollyRogerBronze,    4045595568) \  // TODO

enum class AchievementType : uint32_t
{
#define X(name, value) name = (value),
	ACHIEVEMENT_TYPES
#undef X
};

[[clang::no_destroy]] static const std::unordered_map<AchievementType, std::string_view> g_achievementNames =
{
#define X(name, value) { AchievementType::name, #name },
	ACHIEVEMENT_TYPES
#undef X
};
PA_JSON_SERIALIZE_ENUM_MAP(AchievementType, g_achievementNames);

inline std::string_view GetName(AchievementType achievement)
{
	if (g_achievementNames.contains(achievement))
		return g_achievementNames.at(achievement);
	else
		return "Unknown";
}

enum class MatchOutcome
{
	Win,
	Loss,
	Draw,
	Unknown
};
PA_JSON_SERIALIZE_ENUM(MatchOutcome,
{
	{ MatchOutcome::Win, "win" },
	{ MatchOutcome::Loss, "loss" },
	{ MatchOutcome::Draw, "draw" },
	{ MatchOutcome::Unknown, "unknown" }
});

struct ReplaySummary
{
	std::string Hash;
	MatchOutcome Outcome = MatchOutcome::Unknown;
	float DamageDealt = 0.0f;
	float DamageTaken = 0.0f;
	float DamageSpotting = 0.0f;
	float DamagePotential = 0.0f;
	std::unordered_map<AchievementType, uint32_t> Achievements = {};
	std::unordered_map<RibbonType, uint32_t> Ribbons = {};
};

static inline Core::JsonResult<void> ToJson(rapidjson::Writer<rapidjson::StringBuffer>& writer, const ReplaySummary& s)
{
	writer.StartObject();

	writer.Key("outcome");
	if (!ReplayParser::ToJson(writer, s.Outcome))
		return PA_JSON_ERROR("Failed to write ReplaySummary::Outcome");
	
	writer.Key("damage_dealt");
	writer.Double(s.DamageDealt);
	writer.Key("damage_taken");
	writer.Double(s.DamageTaken);
	writer.Key("damage_spotting");
	writer.Double(s.DamageSpotting);
	writer.Key("damage_potential");
	writer.Double(s.DamagePotential);

	writer.Key("achievements");

	if (!Core::ToJson(writer, s.Achievements))
		return PA_JSON_ERROR("Failed to write ReplaySummary::Achievements");

	writer.Key("ribbons");
	if (!Core::ToJson(writer, s.Ribbons))
		return PA_JSON_ERROR("Failed to write ReplaySummary::Ribbons");

	writer.EndObject();

	return {};
}

[[maybe_unused]] static Core::JsonResult<void> FromJson(std::string_view json, ReplaySummary& s)
{
	PA_TRY(j, Core::ParseJson(json));

	if (j.HasMember("outcome"))
		ReplayParser::FromJson(j["outcome"], s.Outcome);
	
	if (j.HasMember("damage_dealt") && j["damage_dealt"].IsFloat())
		s.DamageDealt = j["damage_dealt"].GetFloat();
	
	if (j.HasMember("damage_taken") && j["damage_taken"].IsFloat())
		s.DamageTaken = j["damage_taken"].GetFloat();
	
	if (j.HasMember("damage_spotting") && j["damage_spotting"].IsFloat())
		s.DamageSpotting = j["damage_spotting"].GetFloat();

	if (j.HasMember("damage_potential") && j["damage_potential"].IsFloat())
		s.DamagePotential = j["damage_potential"].GetFloat();

	if (j.HasMember("achievements"))
	{
		PA_TRYV(Core::FromJson(j["achievements"], s.Achievements));
	}

	if (j.HasMember("ribbons"))
	{
		PA_TRYV(Core::FromJson(j["ribbons"], s.Ribbons));
	}

	return {};
}

class Replay
{
public:
	std::string MetaString;
	ReplayMeta Meta;
	std::vector<PacketType> Packets;
	std::vector<EntitySpec> Specs;

	static ReplayResult<Replay> FromFile(const std::filesystem::path& filePath, const std::filesystem::path& gameFilePath);
	[[nodiscard]] ReplayResult<ReplaySummary> Analyze() const;

	template<typename P>
	void AddPacketCallback(std::function<void(const P&)> callback)
	{
		m_packetParser.Callbacks.Add(callback);
	}

private:
	PacketParser m_packetParser;
};

ReplayResult<ReplaySummary> AnalyzeReplay(const std::filesystem::path& file, const std::filesystem::path& gameFilePath);
bool HasGameScripts(Version gameVersion, const fs::path& gameFilePath);

}  // namespace PotatoAlert::ReplayParser

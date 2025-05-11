// Copyright 2025 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Preprocessor.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <optional>

namespace PotatoAlert::ReplayParser {

// missing RocketRicochet
#define RIBBON_TYPES                              \
	X(AcousticHit, -99, AcousticHit)              \
	X(SonarHit, -98, AcousticHit)                 \
	X(UnknownRibbon, -1, UnknownRibbon)           \
	X(Artillery, 0, Artillery)                    \
	X(TorpedoHit, 1, TorpedoHit)                  \
	X(Bomb, 2, Bomb)                              \
	X(PlaneShotDown, 3, PlaneShotDown)            \
	X(Incapacitation, 4, Incapacitation)          \
	X(Destroyed, 5, Destroyed)                    \
	X(SetFire, 6, SetFire)                        \
	X(Flooding, 7, Flooding)                      \
	X(Citadel, 8, Citadel)                        \
	X(Defended, 9, Defended)                      \
	X(Captured, 10, Captured)                     \
	X(AssistedInCapture, 11, AssistedInCapture)   \
	X(Suppressed, 12, Suppressed)                 \
	X(SecondaryHit, 13, SecondaryHit)             \
	X(OverPenetration, 14, Artillery)             \
	X(Penetration, 15, Artillery)                 \
	X(NonPenetration, 16, Artillery)              \
	X(Ricochet, 17, Artillery)                    \
	X(BuildingDestroyed, 18, BuildingDestroyed)   \
	X(Spotted, 19, Spotted)                       \
	X(DiveBombOverPenetration, 20, Bomb)          \
	X(DiveBombPenetration, 21, Bomb)              \
	X(DiveBombNonPenetration, 22, Bomb)           \
	X(DiveBombRicochet, 23, Bomb)                 \
	X(Rocket, 24, Rocket)                         \
	X(RocketPenetration, 25, Rocket)              \
	X(RocketNonPenetration, 26, Rocket)           \
	X(RocketTorpedoProtectionHit, 30, Rocket)     \
	X(ShotDownByAircraft, 27, ShotDownByAircraft) \
	X(TorpedoProtectionHit, 28, Artillery)        \
	X(BombBulge, 29, Bomb)                        \
	X(DepthChargeHit, 31, DepthChargeHit)         \
	X(BuffSeized, 33, BuffSeized)                 \
	X(SonarOneHit, 39, SonarHit)                  \
	X(SonarTwoHits, 40, SonarHit)                 \
	X(SonarNeutralized, 41, SonarHit)

enum class RibbonType : int8_t
{
#define X(name, value, parent) name = (value),
	RIBBON_TYPES
#undef X
};

inline std::string_view GetName(RibbonType ribbon)
{
	switch (ribbon)
	{
		using enum RibbonType;
#define X(name, value, parent) case name: return PA_STR(name);
	RIBBON_TYPES
#undef X
	}
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

inline std::string_view GetName(AchievementType achievement)
{
	switch (achievement)
	{
		using enum AchievementType;
#define X(name, value) case name: return PA_STR(name);
		ACHIEVEMENT_TYPES
#undef X
	}
	return "Unknown";
}

enum class MatchOutcome
{
	Win,
	Loss,
	Draw,
	Unknown
};

enum class TeamType
{
	Ally,
	Enemy
};

struct EntityStats
{
	std::string Name;
	std::string Clan;
	TeamType Team;

	float DamageDealt;
	float DamagePotential;
	float DamageSpotting;
	float DamageTaken = 0.0f;
	uint32_t Exp;
	uint32_t Frags;
	std::unordered_map<AchievementType, uint32_t> Achievements;
};

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

	std::optional<std::unordered_map<int64_t, EntityStats>> TeamScore;  // Key is vehicleId
};

PA_API Core::JsonResult<ReplaySummary> ReadJson(std::string_view str);
PA_API Core::JsonResult<std::string> WriteJson(const ReplaySummary& summary);

}  // namespace PotatoAlert::ReplayParser

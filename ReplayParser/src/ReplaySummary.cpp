// Copyright 2025 <github.com/razaqq>

#include "Core/Json.hpp"

#include "ReplayParser/ReplaySummary.hpp"

#include <glaze/glaze.hpp>

#include <string>


using PotatoAlert::Core::JsonResult;
using PotatoAlert::ReplayParser::AchievementType;
using PotatoAlert::ReplayParser::EntityStats;
using PotatoAlert::ReplayParser::MatchOutcome;
using PotatoAlert::ReplayParser::ReplaySummary;
using PotatoAlert::ReplayParser::RibbonType;
using PotatoAlert::ReplayParser::TeamType;

template<>
struct glz::meta<TeamType>
{
	using enum TeamType;
	static constexpr auto value = enumerate
	(
		"ally", Ally,
		"enemy", Enemy
	);
};

template<>
struct glz::meta<EntityStats>
{
	using T = EntityStats;
	static constexpr auto value = glz::object
	(
		"name", &T::Name,
		"clan", &T::Clan,
		"team", &T::Team,
		"damage_dealt", &T::DamageDealt,
		"damage_potential", &T::DamagePotential,
		"damage_spotting", &T::DamageSpotting,
		"damage_taken", &T::DamageTaken,
		"exp", &T::Exp,
		"frags", &T::Frags,
		"achievements", &T::Achievements
	);
};

template<>
struct glz::meta<MatchOutcome>
{
	using enum MatchOutcome;
	static constexpr auto value = enumerate
	(
		"win", Win,
		"loss", Loss,
		"draw", Draw,
		"unknown", Unknown
	);
};

template<>
struct glz::meta<AchievementType>
{
	using enum AchievementType;
	static constexpr auto value = enumerate(
		"UnknownAchievement", UnknownAchievement,
		"DevastatingStrike", DevastatingStrike,
		"HighCaliber", HighCaliber,
		"AADefenseExpert", AADefenseExpert,
		"ItsJustAFleshWound", ItsJustAFleshWound,
		"Dreadnought", Dreadnought,
		"Confederate", Confederate,
		"KrakenUnleashed", KrakenUnleashed,
		"FirstBlood", FirstBlood,
		"CloseQuartersExpert", CloseQuartersExpert,
		"Detonation", Detonation,
		"ShoulderToShoulder", ShoulderToShoulder,
		"Fireproof", Fireproof,
		"DieHard", DieHard,
		"SoloWarrior", SoloWarrior,
		"Witherer", Witherer,
		"DoubleStrike", DoubleStrike,
		"Arsionist", Arsionist,
		"StrikeTeam", StrikeTeam,
		"GeneralOffensive", GeneralOffensive,
		"CoordinatedAttack", CoordinatedAttack,
		"BrothersInArms", BrothersInArms,
		"ClanBrawlTop1000", ClanBrawlTop1000,
		"ClanBrawlTop100", ClanBrawlTop100,
		"ClanBrawlTop10", ClanBrawlTop10,
		"ClanBrawlTop1", ClanBrawlTop1,
		"JollyRoger", JollyRoger,
		"Unsinkable", Unsinkable,
		"NaturalSelection", NaturalSelection,
		"Ray", Ray,
		"SeaStar", SeaStar,
		"UniversalSeaman", UniversalSeaman,
		"WeatherBeaten", WeatherBeaten,
		"OldTimer", OldTimer,
		"ExperiencedOne", ExperiencedOne,
		"ImportantMissions", ImportantMissions,
		"SpecialOrders", SpecialOrders,
		"SecretInstructions", SecretInstructions,
		"Exterminator", Exterminator,
		"Raider", Raider,
		"Ravager", Ravager,
		"Shield", Shield,
		"Guardian", Guardian,
		"Protector", Protector,
		"AnInstantBeforeVictory", AnInstantBeforeVictory,
		"SaveCommanderJenkins", SaveCommanderJenkins,
		"CrashTester", CrashTester,
		"SharkAmongShrimps", SharkAmongShrimps,
		"InsertCoin", InsertCoin,
		"MajorContribution", MajorContribution,
		"Assistant", Assistant,
		"WillToWin", WillToWin,
		"TacticalExpertise", TacticalExpertise,
		"CombatScout", CombatScout
	);
};

template<>
struct glz::meta<RibbonType>
{
	using enum RibbonType;
	static constexpr auto value = enumerate(
		"AcousticHit", AcousticHit,
		"SonarHit", SonarHit,
		"UnknownRibbon", UnknownRibbon,
		"Artillery", Artillery,
		"TorpedoHit", TorpedoHit,
		"Bomb", Bomb,
		"PlaneShotDown", PlaneShotDown,
		"Incapacitation", Incapacitation,
		"Destroyed", Destroyed,
		"SetFire", SetFire,
		"Flooding", Flooding,
		"Citadel", Citadel,
		"Defended", Defended,
		"Captured", Captured,
		"AssistedInCapture", AssistedInCapture,
		"Suppressed", Suppressed,
		"SecondaryHit", SecondaryHit,
		"OverPenetration", OverPenetration,
		"Penetration", Penetration,
		"NonPenetration", NonPenetration,
		"Ricochet", Ricochet,
		"BuildingDestroyed", BuildingDestroyed,
		"Spotted", Spotted,
		"DiveBombOverPenetration", DiveBombOverPenetration,
		"DiveBombPenetration", DiveBombPenetration,
		"DiveBombNonPenetration", DiveBombNonPenetration,
		"DiveBombRicochet", DiveBombRicochet,
		"Rocket", Rocket,
		"RocketPenetration", RocketPenetration,
		"RocketNonPenetration", RocketNonPenetration,
		"RocketTorpedoProtectionHit", RocketTorpedoProtectionHit,
		"ShotDownByAircraft", ShotDownByAircraft,
		"TorpedoProtectionHit", TorpedoProtectionHit,
		"BombBulge", BombBulge,
		"DepthChargeHit", DepthChargeHit,
		"BuffSeized", BuffSeized,
		"SonarOneHit", SonarOneHit,
		"SonarTwoHits", SonarTwoHits,
		"SonarNeutralized", SonarNeutralized
	);
};

template<>
struct glz::meta<ReplaySummary>
{
	using T = ReplaySummary;
	static constexpr auto value = glz::object
	(
		"hash", &T::Hash,
		"outcome", &T::Outcome,
		"damage_dealt", &T::DamageDealt,
		"damage_taken", &T::DamageTaken,
		"damage_spotting", &T::DamageSpotting,
		"damage_potential", &T::DamagePotential,
		"achievements", &T::Achievements,
		"ribbons", &T::Ribbons,
		"team_score", &T::TeamScore
	);
};

JsonResult<ReplaySummary> PotatoAlert::ReplayParser::ReadJson(std::string_view str)
{
	return Core::Json::Read<ReplaySummary>(str);
}

JsonResult<std::string> PotatoAlert::ReplayParser::WriteJson(const ReplaySummary& summary)
{
	return Core::Json::Write(summary);
}

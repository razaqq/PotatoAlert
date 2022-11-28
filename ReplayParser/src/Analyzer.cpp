// Copyright 2021 <github.com/razaqq>

#include "Core/Instrumentor.hpp"
#include "Core/Sha256.hpp"

#include "ReplayAnalyzerRust.hpp"
#include "Core/Bytes.hpp"
#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/ReplayParser.hpp"
#include "ReplayParser/Types.hpp"

#include <any>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>


using PotatoAlert::Core::Byte;
using PotatoAlert::ReplayParser::Replay;
using namespace PotatoAlert::ReplayParser;
using namespace PotatoAlert::ReplayAnalyzer;

namespace {

template<typename V>
const std::type_info& VariantType(const V& v)
{
	return std::visit([](auto&& x) -> decltype(auto)
	{
		return typeid(x);
	}, v);
}

template<typename T>
static constexpr void VariantGet(const ArgValue& value, auto then)
{
	if (const T* Name = std::get_if<T>(&value))
	{
		then(*Name);
	}
	else
	{
		LOG_ERROR("Failed to get type '{}' from ArgValue", typeid(T).name());
	}
}

template<typename T>
static constexpr void VariantGet(const EntityMethodPacket& packet, size_t index, auto then)
{
	if (packet.Values.size() <= index)
	{
		LOG_ERROR("Index out of range for EntityMethodPacket '{}' (index {}, size {})",
			packet.MethodName, index, packet.Values.size());
		return;
	}

	if (const T* Name = std::get_if<T>(&packet.Values[index]))
	{
		then(*Name);
	}
	else
	{
		LOG_ERROR("ValueType (index {}) for EntityMethodPacket '{}' did not match '{}' and instead was '{}'",
			index, packet.MethodName, typeid(T).name(), VariantType(packet.Values[index]).name());
	}
}

}

enum class DamageFlag : int64_t
{
	EnemyDamage     = 0,
	AllyDamage      = 1,
	SpottingDamage  = 2,
	PotentialDamage = 3,
};

enum class CameraMode : uint32_t
{
	OverheadMap        = 0,
	FollowingShells    = 1,
	FollowingPlanes    = 2,
	FollowingShip      = 3,
	FollowingSubmarine = 4,
	FreeFlying         = 5,
};

enum class Consumable : int8_t
{
	DamageControl            = 0,
	SpottingAircraft         = 1,
	DefensiveAntiAircraft    = 2,
	SpeedBoost               = 3,
	RepairParty              = 4,
	CatapultFighter          = 5,
	MainBatteryReloadBooster = 6,
	TorpedoReloadBooster     = 7,
	Smoke                    = 8,
	Radar                    = 9,
	HydroacousticSearch      = 10,
	Hydrophone               = 11,
	EnhancedRudders          = 12,
	ReserveBattery           = 13,
};

std::optional<ReplaySummary> Replay::Analyze() const
{
	PA_PROFILE_FUNCTION();

	struct
	{
		std::optional<int8_t> winningTeam = std::nullopt;
		std::optional<int8_t> playerTeam = std::nullopt;
		int32_t PlayerEntityId;
		int64_t PlayerAvatarId;
		int64_t PlayerShipId;
		int64_t PlayerId;
		std::unordered_map<DamageType, float> DamageDealt;
		std::unordered_map<DamageType, float> DamagePotential;
		std::unordered_map<DamageType, float> DamageSpotting;
		float DamageTaken = 0.0f;
		std::unordered_map<RibbonType, uint32_t> Ribbons;
		std::unordered_map<AchievementType, uint32_t> Achievements;
	} replayData;

	for (const PacketType& pak : packets)
	{
		const bool success = std::visit([&replayData, this]<typename Type>(Type&& packet) -> bool
		{
			using T = std::decay_t<Type>;

			if constexpr (std::is_same_v<T, BasePlayerCreatePacket>)
			{
				replayData.PlayerEntityId = packet.EntityId;
			}

			if constexpr (std::is_same_v<T, EntityMethodPacket>)
			{
				if (packet.MethodName == "onArenaStateReceived")
				{
					bool found = false;
					VariantGet<std::vector<Byte>>(packet, 3, [&replayData, &found, this](const std::vector<Byte>& data)
					{
						OnArenaStateReceivedPlayerResult result = ParseArenaStateReceivedPlayers(data, meta.ClientVersionFromExe.GetRaw());

						if (result.IsError)
						{
							LOG_ERROR(result.Error.c_str());
						}

						for (const auto& player : result.Value)
						{
							if (player.EntityId == replayData.PlayerEntityId)
							{
								found = true;
								// replayData.PlayerAvatarId = player.avatarid;
								replayData.PlayerId = player.Id;
								replayData.PlayerShipId = player.ShipId;
							}
						}
					});

					if (!found)
					{
						LOG_ERROR("onArenaStateReceived did not include the player id {} itself", replayData.PlayerEntityId);
						return false;
					}
				}

				if (packet.MethodName == "onBattleEnd")
				{
					// second arg uint8_t winReason
					VariantGet<int8_t>(packet, 0, [&replayData](int8_t team)
					{
						replayData.winningTeam = team;
					});
				}

				if (packet.MethodName == "receiveDamageStat")
				{
					if (packet.Values.size() != 1)
						return false;

					VariantGet<std::vector<Byte>>(packet, 0, [&replayData, &packet](const std::vector<Byte>& data)
					{
						ReceiveDamageStatResult result = ParseReceiveDamageStat(data);

						if (result.IsError)
						{
							LOG_ERROR(result.Error.c_str());
						}

						for (const ReceiveDamageStat& stat : result.Value)
						{
							const DamageType dmgType = static_cast<DamageType>(stat.DamageType);
							switch (static_cast<DamageFlag>(stat.DamageFlag))
							{
								case DamageFlag::EnemyDamage:
								{
									replayData.DamageDealt[dmgType] = stat.Damage;
									break;
								}
								case DamageFlag::PotentialDamage:
								{
									replayData.DamagePotential[dmgType] = stat.Damage;
									break;
								}
								case DamageFlag::SpottingDamage:
								{
									replayData.DamageSpotting[dmgType] = stat.Damage;
									break;
								}
								default:
									break;
							}
						}
					});
				}

				if (packet.MethodName == "receiveDamagesOnShip")
				{
					if (packet.EntityId != replayData.PlayerShipId)
					{
						return true;
					}

					VariantGet<std::vector<ArgValue>>(packet, 0, [&replayData, &packet](const std::vector<ArgValue>& vec)
					{
						for (const ArgValue& elem : vec)
						{
							VariantGet<std::unordered_map<std::string, ArgValue>>(elem, [&replayData, &packet](const std::unordered_map<std::string, ArgValue>& dict)
							{
								// other field is 'vehicleID' int32_t of the aggressor
								if (dict.contains("damage"))
								{
									VariantGet<float>(dict.at("damage"), [&replayData](float damage)
									{
										replayData.DamageTaken += damage;
									});
								}
							});
						}
					});
				}

				if (packet.MethodName == "onRibbon")
				{
					VariantGet<int8_t>(packet, 0, [&replayData](int8_t value)
					{
						const RibbonType ribbon = static_cast<RibbonType>(value);
						if (replayData.Ribbons.contains(ribbon))
						{
							replayData.Ribbons[ribbon] += 1;
						}
						else
						{
							replayData.Ribbons[ribbon] = 1;
						}
					});
				}

				if (packet.MethodName == "onAchievementEarned")
				{
					bool discard = true;
					VariantGet<int32_t>(packet, 0, [&replayData, &discard, this](int32_t id)
					{
						// since version 0.11.4 this is a different id
						if (meta.ClientVersionFromExe >= Version(0, 11, 4))
						{
							if (id == replayData.PlayerId)
							{
								discard = false;
							}
						}
						else
						{
							if (id == replayData.PlayerEntityId)
							{
								discard = false;
							}
						}
					});
					VariantGet<uint32_t>(packet, 1, [&replayData, discard](uint32_t value)
					{
						if (discard)
							return;
						const AchievementType achievement = static_cast<AchievementType>(value);
						if (replayData.Achievements.contains(achievement))
						{
							replayData.Achievements[achievement] += 1;
						}
						else
						{
							replayData.Achievements[achievement] = 1;
						}
					});
				}
			}

			if constexpr (std::is_same_v<T, CellPlayerCreatePacket>)
			{
				if (packet.Values.contains("teamId"))
				{
					VariantGet<int8_t>(packet.Values.at("teamId"), [&replayData](int8_t team)
					{
						replayData.playerTeam = team;
					});
				}
			}

			return true;
		}, pak);

		if (!success)
		{
			LOG_ERROR("Error while analyzing replay packets");
			return {};
		}
	}

	auto damageDealtValues = std::views::values(replayData.DamageDealt);
	float damageDealt = std::accumulate(damageDealtValues.begin(), damageDealtValues.end(), 0.0f);

	auto dmgPotentialValues = std::views::values(replayData.DamagePotential);
	float damagePotential = std::accumulate(dmgPotentialValues.begin(), dmgPotentialValues.end(), 0.0f);

	auto dmgSpottingValues = std::views::values(replayData.DamageSpotting);
	float damageSpotting = std::accumulate(dmgSpottingValues.begin(), dmgSpottingValues.end(), 0.0f);

	MatchOutcome outcome;

	if (!replayData.playerTeam || !replayData.winningTeam)
	{
		LOG_TRACE("Failed to determine match outcome, PT {} WT {}", replayData.playerTeam.has_value(), replayData.winningTeam.has_value());
		outcome = MatchOutcome::Unknown;
	}
	else if (replayData.playerTeam.value() == replayData.winningTeam.value())
	{
		outcome = MatchOutcome::Win;
	}
	else if (replayData.winningTeam.value() == -1)
	{
		outcome = MatchOutcome::Draw;
	}
	else
	{
		outcome = MatchOutcome::Loss;
	}

	std::string hash;
	if (!Core::Sha256(metaString, hash))
	{
		LOG_ERROR("Failed to get SHA256 hash of replay meta");
		return {};
	}

	return ReplaySummary
	{
		.Hash = hash,
		.Outcome = outcome,
		.DamageDealt = damageDealt,
		.DamageTaken = replayData.DamageTaken,
		.DamageSpotting = damageSpotting,
		.DamagePotential = damagePotential,
		.Achievements = replayData.Achievements,
		.Ribbons = replayData.Ribbons,
	};
}

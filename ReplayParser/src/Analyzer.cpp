// Copyright 2021 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Instrumentor.hpp"
#include "Core/Sha256.hpp"

#include "ReplayAnalyzerRust.hpp"
#include "ReplayParser/GameFiles.hpp"
#include "ReplayParser/ReplayParser.hpp"
#include "ReplayParser/Result.hpp"
#include "ReplayParser/Types.hpp"
#include "ReplayParser/Variant.hpp"

#include <any>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>


using PotatoAlert::Core::Byte;
using PotatoAlert::ReplayParser::Replay;
using PotatoAlert::ReplayParser::ReplayResult;
using namespace PotatoAlert::ReplayParser;
using namespace PotatoAlert::ReplayAnalyzer;

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

ReplayResult<ReplaySummary> Replay::Analyze() const
{
	PA_PROFILE_FUNCTION();

	struct
	{
		std::optional<int8_t> winningTeam = std::nullopt;
		std::optional<int8_t> playerTeam = std::nullopt;
		int32_t PlayerEntityId;
		// int64_t PlayerAvatarId;
		int64_t PlayerShipId;
		int64_t PlayerId;
		std::unordered_map<DamageType, float> DamageDealt;
		std::unordered_map<DamageType, float> DamagePotential;
		std::unordered_map<DamageType, float> DamageSpotting;
		float DamageTaken = 0.0f;
		std::unordered_map<RibbonType, uint32_t> Ribbons;
		std::unordered_map<AchievementType, uint32_t> Achievements;
	} replayData;

	for (const PacketType& pak : Packets)
	{
		const ReplayResult<void> packetResult = std::visit([&replayData, this]<typename Type>(Type&& packet) -> ReplayResult<void>
		{
			using T = std::decay_t<Type>;

			if constexpr (std::is_same_v<T, BasePlayerCreatePacket>)
			{
				replayData.PlayerEntityId = packet.EntityId;

				return {};
			}

			if constexpr (std::is_same_v<T, EntityMethodPacket>)
			{
				if (packet.MethodName == "onArenaStateReceived")
				{
					bool found = false;
					PA_TRYV(VariantGet<std::vector<Byte>>(packet, 3, [&replayData, &found, this](const std::vector<Byte>& data) -> ReplayResult<void>
					{
						OnArenaStateReceivedPlayerResult result = ParseArenaStateReceivedPlayers(data, Meta.ClientVersionFromExe.GetRaw());

						if (result.IsError)
						{
							return PA_REPLAY_ERROR("{}", result.Error.c_str());
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

						return {};
					}));

					if (!found)
					{
						return PA_REPLAY_ERROR("onArenaStateReceived did not include the player id {} itself", replayData.PlayerEntityId);
					}

					return {};
				}

				if (packet.MethodName == "onBattleEnd")
				{
					if (Meta.ClientVersionFromExe < Version(12, 5, 0))
					{
						// second arg uint8_t winReason
						return VariantGet<int8_t>(packet, 0, [&replayData](int8_t team) -> ReplayResult<void>
						{
							replayData.winningTeam = team;

							return {};
						});
					}
				}

				if (packet.MethodName == "receiveDamageStat")
				{
					if (packet.Values.size() != 1)
					{
						return PA_REPLAY_ERROR("receiveDamageStat Values were not size 1");
					}

					return VariantGet<std::vector<Byte>>(packet, 0, [&replayData, &packet](const std::vector<Byte>& data) -> ReplayResult<void>
					{
						ReceiveDamageStatResult result = ParseReceiveDamageStat(data);

						if (result.IsError)
						{
							return PA_REPLAY_ERROR("Failed to parse damage stat: {}", result.Error.c_str());
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

						return {};
					});
				}

				if (packet.MethodName == "receiveDamagesOnShip")
				{
					if (packet.EntityId != replayData.PlayerShipId)
					{
						return {};  // just ignore this packet if the ids dont match
					}

					return VariantGet<std::vector<ArgValue>>(packet, 0, [&replayData, &packet](const std::vector<ArgValue>& vec) -> ReplayResult<void>
					{
						for (const ArgValue& elem : vec)
						{
							VariantGet<std::unordered_map<std::string, ArgValue>>(elem, [&replayData, &packet](const std::unordered_map<std::string, ArgValue>& dict) -> ReplayResult<void>
							{
								// other field is 'vehicleID' int32_t of the aggressor
								if (dict.contains("damage"))
								{
									VariantGet<float>(dict.at("damage"), [&replayData](float damage) -> ReplayResult<void>
									{
										replayData.DamageTaken += damage;
										return {};
									});
								}
								return {};
							});
						}

						return {};
					});

					return {};
				}

				// until 12.0.0, since then its an EntityProperty
				if (Meta.ClientVersionFromExe < Version(12, 0, 0))
				{
					if (packet.MethodName == "onRibbon")
					{
						return VariantGet<int8_t>(packet, 0, [&replayData](int8_t value) -> ReplayResult<void>
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

							return {};
						});
					}
				}

				if (packet.MethodName == "onAchievementEarned")
				{
					bool discard = true;
					PA_TRYV(VariantGet<int32_t>(packet, 0, [&replayData, &discard, this](int32_t id) -> ReplayResult<void>
					{
						// since version 0.11.4 this is a different id
						if (Meta.ClientVersionFromExe >= Version(0, 11, 4))
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
						return {};
					}));
					PA_TRYV(VariantGet<uint32_t>(packet, 1, [&replayData, discard](uint32_t value) -> ReplayResult<void>
					{
						if (discard)
							return {};
						const AchievementType achievement = static_cast<AchievementType>(value);
						if (replayData.Achievements.contains(achievement))
						{
							replayData.Achievements[achievement] += 1;
						}
						else
						{
							replayData.Achievements[achievement] = 1;
						}
						return {};
					}));

					return {};
				}
			}

			if constexpr (std::is_same_v<T, CellPlayerCreatePacket>)
			{
				if (packet.Values.contains("teamId"))
				{
					VariantGet<int8_t>(packet.Values.at("teamId"), [&replayData](int8_t team) -> ReplayResult<void>
					{
						replayData.playerTeam = team;
						return {};
					});
				}

				return {};
			}

			return {};
		}, pak);
		PA_TRYV(packetResult);
	}

	auto damageDealtValues = std::views::values(replayData.DamageDealt);
	float damageDealt = std::accumulate(damageDealtValues.begin(), damageDealtValues.end(), 0.0f);

	auto dmgPotentialValues = std::views::values(replayData.DamagePotential);
	float damagePotential = std::accumulate(dmgPotentialValues.begin(), dmgPotentialValues.end(), 0.0f);

	auto dmgSpottingValues = std::views::values(replayData.DamageSpotting);
	float damageSpotting = std::accumulate(dmgSpottingValues.begin(), dmgSpottingValues.end(), 0.0f);

	MatchOutcome outcome;

	// since 12.0.0
	if (Meta.ClientVersionFromExe >= Version(12, 0, 0))
	{
		if (!m_packetParser.Entities.contains(replayData.PlayerEntityId))
		{
			return PA_REPLAY_ERROR("PacketParser has no entity for PlayerEntityId");
		}
		const Entity& playerEntity = m_packetParser.Entities.at(replayData.PlayerEntityId);
		if (!playerEntity.ClientPropertiesValues.contains("privateVehicleState"))
		{
			return PA_REPLAY_ERROR("Player entity is missing ClientProperty 'privateVehicleState'");
		}
		const ArgValue& privateVehicleState = playerEntity.ClientPropertiesValues.at("privateVehicleState");

		PA_TRYV(VariantGet<std::unordered_map<std::string, ArgValue>>(privateVehicleState, [&replayData](auto& state) -> ReplayResult<void>
		{
			if (!state.contains("ribbons"))
			{
				return PA_REPLAY_ERROR("privateVehicleState is missing key 'ribbons'");
			}
			return VariantGet<std::vector<ArgValue>>(state.at("ribbons"), [&replayData](auto& ribbons) -> ReplayResult<void>
			{
				for (const ArgValue& ribbonValue : ribbons)
				{
					PA_TRYV(VariantGet<std::unordered_map<std::string, ArgValue>>(ribbonValue, [&replayData](auto& ribbon) -> ReplayResult<void>
					{
						if (!ribbon.contains("count"))
						{
							return PA_REPLAY_ERROR("ribbon is missing key 'count'");
						}
						uint32_t ribbonCount;
						PA_TRYV(VariantGet<uint16_t>(ribbon.at("count"), [&ribbonCount](uint16_t count) -> ReplayResult<void>
						{
							ribbonCount = count;
							return {};
						}));

						if (!ribbon.contains("ribbonId"))
						{
							return PA_REPLAY_ERROR("ribbon is missing key 'ribbonId'");
						}
						RibbonType ribbonType;
						PA_TRYV(VariantGet<int8_t>(ribbon.at("ribbonId"), [&ribbonType](int8_t ribbonId) -> ReplayResult<void>
						{
							ribbonType = static_cast<RibbonType>(ribbonId);
							return {};
						}));
						replayData.Ribbons.emplace(ribbonType, ribbonCount);
						return {};
					}));
				}
				return {};
			});
		}));
	}

	if (Meta.ClientVersionFromExe >= Version(12, 5, 0))
	{
		const auto battleLogic = std::ranges::find_if(m_packetParser.Entities | std::views::values, [](const Entity& entity)
		{
			return entity.Spec.get().Name == "BattleLogic";
		});

		if (battleLogic == std::end(m_packetParser.Entities | std::views::values))
		{
			return PA_REPLAY_ERROR("No entity with spec BattleLogic");
		}

		if (!(*battleLogic).ClientPropertiesValues.contains("battleResult"))
		{
			return PA_REPLAY_ERROR("Entity BattleLogic is missing 'battleResult'");
		}

		PA_TRYV(VariantGet<std::unordered_map<std::string, ArgValue>>((*battleLogic).ClientPropertiesValues.at("battleResult"), [&replayData](const auto& map) -> ReplayResult<void>
		{
			if (map.contains("winnerTeamId"))
			{
				PA_TRYV(VariantGet<int8_t>(map.at("winnerTeamId"), [&replayData](int8_t winnerTeamId) -> ReplayResult<void>
				{
					replayData.winningTeam = winnerTeamId;
					return {};
				}));

				return {};
			}

			return PA_REPLAY_ERROR("battleResult did not contain 'winnerTeamId'");
		}));
	}

	if (!replayData.playerTeam || !replayData.winningTeam || (replayData.winningTeam && replayData.winningTeam == -2))
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
	if (!Core::Sha256(MetaString, hash))
	{
		return PA_REPLAY_ERROR("Failed to get SHA256 hash of replay meta");
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

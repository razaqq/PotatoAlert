// Copyright 2021 <github.com/razaqq>

#include "Core/Bytes.hpp"
#include "Core/Instrumentor.hpp"
#include "Core/String.hpp"
#include "Core/Sha256.hpp"
#include "Core/Version.hpp"

#include "ReplayAnalyzerRust.hpp"
#include "ReplayParser/ArgValueReader.hpp"
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
using PotatoAlert::Core::Version;
using PotatoAlert::ReplayParser::Replay;
using PotatoAlert::ReplayParser::ReplayResult;
using namespace PotatoAlert::ReplayParser;
using namespace PotatoAlert::ReplayAnalyzer;
namespace fs = std::filesystem;

using AccountDbId64 = int64_t;
using EntityId64 = int64_t;
using PlayerId64 = int64_t;
using VehicleId64 = int64_t;
using TeamId = int8_t;

ReplayResult<ReplaySummary> Replay::Analyze(const fs::path& scriptsPath) const
{
	PA_PROFILE_FUNCTION();

	struct PlayerMeta
	{
		std::string Name;
		std::string ClanTag;
		PlayerId64 Id;
		VehicleId64 VehicleId;
		EntityId64 EntityId;
		TeamId TeamId;
	};

	struct
	{
		std::optional<TeamId> WinningTeam = std::nullopt;
		std::optional<TeamId> PlayerTeam = std::nullopt;
		TypeEntityId PlayerEntityId;
		// int64_t PlayerAvatarId;
		int64_t PlayerShipId;
		int64_t PlayerId;
		std::unordered_map<DamageType, float> DamageDealt;
		std::unordered_map<DamageType, float> DamagePotential;
		std::unordered_map<DamageType, float> DamageSpotting;
		float DamageTaken = 0.0f;
		std::unordered_map<RibbonType, uint32_t> Ribbons;

		std::unordered_map<AccountDbId64, PlayerMeta> VehicleMap;
		std::unordered_map<VehicleId64, EntityStats> EntityStats;
	} replayData;

	const auto parser = MakePacketParser(
		On<EntityCreatePacket>([](const EntityCreatePacket&) -> ReplayResult<void> { return {}; }),
		On<EntityPropertyPacket>([](const EntityPropertyPacket&) -> ReplayResult<void> { return {}; }),
		On<NestedPropertyUpdatePacket>([](const NestedPropertyUpdatePacket&) -> ReplayResult<void> { return {}; }),
		On<BasePlayerCreatePacket>([&replayData](const BasePlayerCreatePacket& packet) -> ReplayResult<void>
		{
			replayData.PlayerEntityId = packet.EntityId;
			return {};
		}),
		On<EntityMethodPacket>([&replayData, this](const EntityMethodPacket& packet) -> ReplayResult<void>
		{
			if (packet.MethodName == "onArenaStateReceived")
			{
				if (packet.Values.size() < 4)
					return PA_REPLAY_ERROR("Avatar onArenaStateReceived only has {} values", packet.Values.size());

				bool found = false;

				PA_TRY(playerData, GetArgValue<std::vector<Byte>>(packet.Values[3]));
				OnArenaStateReceivedPlayerResult playerResult = ParseArenaStateReceivedPlayers(playerData, Meta.ClientVersionFromExe.GetRaw());
				if (playerResult.IsError)
					return PA_REPLAY_ERROR("Failed to parse onArenaStateReceived players: {}", playerResult.Error.c_str());
				for (const OnArenaStateReceivedPlayer& player : *playerResult.Value)
				{
					replayData.VehicleMap[player.AccoundDbId] = PlayerMeta
					{
						.Name = std::string(player.Name),
						.ClanTag = std::string(player.ClanTag),
						.Id = player.Id,
						.VehicleId = player.ShipId,
						.EntityId = player.EntityId,
						.TeamId = static_cast<TeamId>(player.TeamId),
					};
					replayData.EntityStats[player.ShipId] = EntityStats{};

					if (player.EntityId == replayData.PlayerEntityId)
					{
						found = true;
						// replayData.PlayerAvatarId = player.avatarid;
						replayData.PlayerId = player.Id;
						replayData.PlayerShipId = player.ShipId;
					}
				}

				PA_TRY(botsData, GetArgValue<std::vector<Byte>>(packet.Values[4]));
				OnArenaStateReceivedPlayerResult botsResult = ParseArenaStateReceivedBots(botsData, Meta.ClientVersionFromExe.GetRaw());
				if (botsResult.IsError)
					return PA_REPLAY_ERROR("Failed to parse onArenaStateReceived bots: {}", botsResult.Error.c_str());

				for (const OnArenaStateReceivedPlayer& bot : *botsResult.Value)
				{
					replayData.VehicleMap[bot.AccoundDbId] = PlayerMeta
					{
						.Name = std::string(bot.Name),
						.ClanTag = std::string(bot.ClanTag),
						.Id = bot.Id,
						.VehicleId = bot.ShipId,
						.EntityId = bot.EntityId,
						.TeamId = static_cast<TeamId>(bot.TeamId),
					};
					replayData.EntityStats[bot.ShipId] = EntityStats{};
				}

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
					PA_TRYA(replayData.WinningTeam, GetArgValue<TeamId>(packet.Values, { 0 }));
				}
			}

			if (packet.MethodName == "receiveDamageStat")
			{
				if (packet.Values.size() != 1)
				{
					return PA_REPLAY_ERROR("receiveDamageStat Values were not size 1");
				}

				return VariantGet<std::vector<Byte>>(packet, 0, [&replayData](const std::vector<Byte>& data) -> ReplayResult<void>
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
				if (packet.Values.empty())
				{
					return PA_REPLAY_ERROR("receiveDamagesOnShip had no values");
				}

				PA_TRY(damages, GetArgValue<ArrayValue>(packet.Values[0], {}));
				for (const ArgValue& damageValue : damages)
				{
					PA_TRY(aggressor, GetArgValue<int32_t>(damageValue, { "vehicleID" }));
					PA_TRY(damage, GetArgValue<float>(damageValue, { "damage" }));

					if (replayData.EntityStats.contains(aggressor))
					{
						EntityStats& stats = replayData.EntityStats[aggressor];
						stats.DamageDealt += damage;
					}

					if (packet.EntityId == replayData.PlayerShipId)
					{
						replayData.DamageTaken += damage;
					}
				}
			}

			// until 12.0.0, since then it's an EntityProperty
			if (packet.MethodName == "onRibbon" && Meta.ClientVersionFromExe < Version(12, 0, 0))
			{
				PA_TRY(ribbonId, GetArgValue<int8_t>(packet.Values, { 0 }));

				const RibbonType ribbon = static_cast<RibbonType>(ribbonId);
				if (replayData.Ribbons.contains(ribbon))
				{
					replayData.Ribbons[ribbon] += 1;
				}
				else
				{
					replayData.Ribbons[ribbon] = 1;
				}

				return {};
			}

			if (packet.MethodName == "onAchievementEarned")
			{
				PA_TRY(id, GetArgValue<int32_t>(packet.Values, { 0 }));
				PA_TRY(achievementId, GetArgValue<uint32_t>(packet.Values, { 1 }));
				const AchievementType achievement = static_cast<AchievementType>(achievementId);

				const auto players = replayData.VehicleMap | std::views::values;
				const auto it = std::ranges::find_if(players, [this, id](const PlayerMeta& meta)
				{
					// since version 0.11.4 this is a different id
					if (Meta.ClientVersionFromExe >= Version(0, 11, 4))
					{
						return meta.Id == id;
					}
					else
					{
						return meta.EntityId == id;
					}
				});
				if (it != players.end() && replayData.EntityStats.contains((*it).VehicleId))
				{
					replayData.EntityStats.at((*it).VehicleId).Achievements[achievement]++;
				}
				return {};
			}

			return {};
		}),
		On<CellPlayerCreatePacket>([&replayData](const CellPlayerCreatePacket& packet) -> ReplayResult<void>
		{
			if (packet.Values.contains("teamId"))
			{
				PA_TRYA(replayData.PlayerTeam, GetArgValue<TeamId>(packet.Values.at("teamId")));
			}

			return {};
		})
	);

	PA_TRY(ctx, Replay::PrepareContext(scriptsPath, Meta.ClientVersionFromExe));
	PA_TRYV(ParsePackets(ctx, parser));

	auto damageDealtValues = replayData.DamageDealt | std::views::values;
	const float damageDealt = std::accumulate(damageDealtValues.begin(), damageDealtValues.end(), 0.0f);

	auto dmgPotentialValues = replayData.DamagePotential | std::views::values;
	const float damagePotential = std::accumulate(dmgPotentialValues.begin(), dmgPotentialValues.end(), 0.0f);

	auto dmgSpottingValues = replayData.DamageSpotting | std::views::values;
	const float damageSpotting = std::accumulate(dmgSpottingValues.begin(), dmgSpottingValues.end(), 0.0f);

	MatchOutcome outcome;

	// get ribbons from privateVehicleState
	if (Meta.ClientVersionFromExe >= Version(12, 0, 0))
	{
		if (!ctx.Entities.contains(replayData.PlayerEntityId))
		{
			return PA_REPLAY_ERROR("PacketParser has no entity for PlayerEntityId");
		}
		const Entity& playerEntity = ctx.Entities.at(replayData.PlayerEntityId);

		if (!playerEntity.ClientPropertiesValues.contains("privateVehicleState"))
		{
			return PA_REPLAY_ERROR("Player entity is missing ClientProperty 'privateVehicleState'");
		}
		const ArgValue& privateVehicleState = playerEntity.ClientPropertiesValues.at("privateVehicleState");

		PA_TRY(ribbons, GetArgValue<ArrayValue>(privateVehicleState, { "ribbons" }));
		for (const ArgValue& ribbonValue : ribbons)
		{
			PA_TRY(ribbonId, GetArgValue<int8_t>(ribbonValue, { "ribbonId" }));
			PA_TRY(count, GetArgValue<uint16_t>(ribbonValue, { "count" }));
			replayData.Ribbons.emplace(static_cast<RibbonType>(ribbonId), count);
		}
	}

	// determine the winner
	if (Meta.ClientVersionFromExe >= Version(12, 5, 0))
	{
		const auto entities = ctx.Entities | std::views::values;
		const auto battleLogic = std::ranges::find_if(entities, [](const Entity& entity)
		{
			return entity.Spec.get().Name == "BattleLogic";
		});

		if (battleLogic == entities.end())
		{
			return PA_REPLAY_ERROR("No entity with spec BattleLogic");
		}

		if (!(*battleLogic).ClientPropertiesValues.contains("battleResult"))
		{
			return PA_REPLAY_ERROR("Entity BattleLogic is missing 'battleResult'");
		}
		const ArgValue& battleResult = (*battleLogic).ClientPropertiesValues.at("battleResult");
		PA_TRYA(replayData.WinningTeam, GetArgValue<TeamId>(battleResult, { "winnerTeamId" }));
	}

	if (!replayData.PlayerTeam || !replayData.WinningTeam || (replayData.WinningTeam && replayData.WinningTeam == -2))
	{
		//LOG_TRACE("Failed to determine match outcome, PT {} WT {}", replayData.playerTeam.has_value(), replayData.winningTeam.has_value());
		outcome = MatchOutcome::Unknown;
	}
	else if (replayData.PlayerTeam.value() == replayData.WinningTeam.value())
	{
		outcome = MatchOutcome::Win;
	}
	else if (replayData.WinningTeam.value() == -1)
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

	if (!replayData.EntityStats.contains(replayData.PlayerShipId))
	{
		return PA_REPLAY_ERROR("EntityStats do not contain the player vehicle");
	}

	return ReplaySummary
	{
		.Hash = hash,
		.Outcome = outcome,
		.DamageDealt = damageDealt,
		.DamageTaken = replayData.DamageTaken,
		.DamageSpotting = damageSpotting,
		.DamagePotential = damagePotential,
		.Achievements = replayData.EntityStats.at(replayData.PlayerShipId).Achievements,
		.Ribbons = replayData.Ribbons,
		.TeamScore = std::move(replayData.EntityStats),
	};
}

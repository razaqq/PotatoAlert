// Copyright 2025 <github.com/razaqq>

#include "Client/StatsParser.hpp"

#include "Core/Format.hpp"
#include "Core/Json.hpp"

#include <glaze/glaze.hpp>

#include <expected>
#include <string>


using PotatoAlert::Client::StatsParser::Stat;
using PotatoAlert::Client::StatsParser::Ship;
using PotatoAlert::Client::StatsParser::Clan;
using PotatoAlert::Client::StatsParser::Player;
using PotatoAlert::Client::StatsParser::Team;
using PotatoAlert::Client::StatsParser::Match;
using PotatoAlert::Core::JsonError;
using PotatoAlert::Core::JsonResult;

template<>
struct glz::meta<Stat>
{
	using T = Stat;
	static constexpr auto value = object(
		"string", &T::Str,
		"color", &T::ColorRGB
	);
};

template<>
struct glz::meta<Clan>
{
	using T = Clan;
	static constexpr auto value = object(
		"name", &T::Name,
		"tag", &T::Tag,
		"color", &T::ColorRGB,
		"region", &T::Region
	);
};

template<>
struct glz::meta<Ship>
{
	using T = Ship;
	static constexpr auto value = object(
		"name", &T::Name,
		"class", &T::Class,
		"nation", &T::Nation,
		"tier", &T::Tier
	);
};

template<>
struct glz::meta<Player>
{
	using T = Player;
	static constexpr auto value = object(
		"clan", &T::Clan,
		"hidden_profile", &T::HiddenPro,
		"name", &T::Name,
		"name_color", &T::NameColor,
		"ship", &T::Ship,
		"battles", &T::Battles,
		"win_rate", &T::Winrate,
		"avg_dmg", &T::AvgDmg,
		"battles_ship", &T::BattlesShip,
		"win_rate_ship", &T::WinrateShip,
		"avg_dmg_ship", &T::AvgDmgShip,
		"karma", &T::Karma,
		"pr_color", &T::PrColor,
		"wows_numbers_link", &T::WowsNumbers,
		"is_using_pa", &T::IsUsingPa
	);
};

template<>
struct glz::meta<Team>
{
	using T = Team;
	static constexpr auto value = object(
		"id", &T::Id,
		"players", &T::Players,
		"avg_dmg", &T::AvgDmg,
		"avg_win_rate", &T::AvgWr
	);
};

template<>
struct glz::meta<Match>
{
	using T = Match;
	static constexpr auto value = object(
		"team1", &T::Team1,
		"team2", &T::Team2,
		"match_group", &T::MatchGroup,
		"stats_mode", &T::StatsMode,
		"region", &T::Region,
		"map", &T::Map,
		"date_time", &T::DateTime
	);
};

JsonResult<Match> PotatoAlert::Client::StatsParser::ParseMatch(std::string_view json)
{
	const auto res = glz::read_json<Match>(json);
	if (!res)
	{
		return std::unexpected<JsonError>(glz::format_error(res, json));
	}
	return *res;
}

JsonResult<std::string> PotatoAlert::Client::StatsParser::ToCSV(const Match& match)
{
#if 0
	const auto res = glz::write_csv(match);
	if (!res)
	{
		return std::unexpected<JsonError>(glz::format_error(res, match));
	}
	return *res;
#endif
	std::string out;
	out += "Team;Player;Clan;Ship;Matches;Winrate;AverageDamage;MatchesShip;WinrateShip;AverageDamageShip;WowsNumbers\n";

	auto getTeam = [&out](const Team& team, std::string_view teamID)
	{
		for (auto& player : team.Players)
		{
			std::string clanName;
			if (player.Clan)
				clanName = player.Clan->Name;

			std::string shipName;
			if (player.Ship)
				shipName = player.Ship->Name;

			out += fmt::format("{};{};{};{};{};{};{};{};{};{};{}\n",
					teamID, player.Name, clanName, shipName,
					player.Battles.Str, player.Winrate.Str,
					player.AvgDmg.Str, player.BattlesShip.Str,
					player.WinrateShip.Str, player.AvgDmgShip.Str,
					player.WowsNumbers);
		}
	};
	getTeam(match.Team1, "1");
	getTeam(match.Team2, "2");

	return out;
}

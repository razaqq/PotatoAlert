// Copyright 2021 <github.com/razaqq>

#include "Client/StatsParser.hpp"

#include "Core/Json.hpp"

#include <QLabel>

#include <array>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>


using namespace PotatoAlert::Client::StatsParser;

static QString ToQString(const std::string_view& str)
{
	return QString::fromUtf8(str.data(), static_cast<int>(str.size()));
}

namespace _JSON {

struct Color
{
	int m_r, m_g, m_b;
	std::optional<int> m_a;

	Color() = default;
	[[maybe_unused]] Color(int r, int g, int b) : m_r(r), m_g(g), m_b(b) {}
	[[maybe_unused]] Color(int r, int g, int b, int a) : m_r(r), m_g(g), m_b(b), m_a(a) {}
	[[maybe_unused]] explicit Color(const std::array<int, 3>& arr) : m_r(arr[0]), m_g(arr[1]), m_b(arr[2]) {}
	[[maybe_unused]] explicit Color(const std::array<int, 4>& arr) : m_r(arr[0]), m_g(arr[1]), m_b(arr[2]), m_a(arr[3]) {}

	[[nodiscard]] std::string ToString() const
	{
		if (!m_a)
			return std::format("rgb({}, {}, {})", m_r, m_g, m_b);
		else
			return std::format("rgba({}, {}, {}, {})", m_r, m_g, m_b, m_a.value());
	}

	[[nodiscard]] bool Valid() const
	{
		auto valid = [](int i) -> bool { return i >= 0 && i <= 255; };
		if (!m_a.has_value())
			return valid(m_r) && valid(m_g) && valid(m_b);
		else
			return valid(m_r) && valid(m_g) && valid(m_b) && valid(m_a.value());
	}

	[[nodiscard]] QColor QColor() const
	{
		if (!m_a)
			return QColor::fromRgb(m_r, m_g, m_b);
		else
			return QColor::fromRgb(m_r, m_g, m_b, m_a.value());
	}
};

struct Stat
{
	std::string string;
	Color color;

	[[nodiscard]] QTableWidgetItem* GetField(const QFont& font, const QColor& bg, const QFlags<Qt::AlignmentFlag>& align) const
	{
		auto item = new QTableWidgetItem(ToQString(this->string));
		item->setForeground(this->color.QColor());
		item->setBackground(bg);
		item->setFont(font);
		item->setTextAlignment(align);
		return item;
	}

	[[nodiscard]] Label GetLabel(const QString& suffix = "") const
	{
		return {ToQString(this->string) + suffix, this->color.QColor()};
	}
};

[[maybe_unused]] static void from_json(const json& j, Stat& s)
{
	j.at("string").get_to(s.string);
	s.color = Color(j.at("color").get<std::array<int, 3>>());
}

struct Clan
{
	std::string name;
	std::string tag;
	Color color;
	std::string region;

	[[nodiscard]] Label GetTagLabel() const
	{
		return { "[" + ToQString(this->tag) + "] ", this->color.QColor() };
	}
	[[nodiscard]] Label GetNameLabel() const
	{
		return { ToQString(this->name), std::nullopt };
	}
	[[nodiscard]] Label GetRegionLabel() const
	{
		return { ToQString(this->region), std::nullopt };
	}
};

[[maybe_unused]] static void from_json(const json& j, Clan& c)
{
	j.at("name").get_to(c.name);
	j.at("tag").get_to(c.tag);
	c.color = Color(j.at("color").get<std::array<int, 3>>());
	j.at("region").get_to(c.region);
}

struct Ship
{
	std::string Name;
	std::string Class;
	std::string Nation;
	uint8_t Tier;
	// Color color;

	[[nodiscard]] QTableWidgetItem* GetField(const QFont& font, const QColor& bg, const QFlags<Qt::AlignmentFlag>& align) const
	{
		auto item = new QTableWidgetItem(ToQString(Name));
		// item->setForeground(this->color.QColor());
		item->setBackground(bg);
		item->setFont(font);
		item->setTextAlignment(align);
		return item;
	}
};

[[maybe_unused]] static void from_json(const json& j, Ship& s)
{
	j.at("name").get_to(s.Name);
	j.at("class").get_to(s.Class);
	j.at("nation").get_to(s.Nation);
	j.at("tier").get_to(s.Tier);
	// s.color = Color(j.at("color").get<std::array<int, 3>>());
}

struct Player
{
	std::optional<Clan> clan;
	bool hiddenPro;
	std::string name;
	Color nameColor;
	std::optional<Ship> ship;
	Stat battles;
	Stat winrate;
	Stat avgDmg;
	Stat battlesShip;
	Stat winrateShip;
	Stat avgDmgShip;
	Color prColor;
	std::string wowsNumbers;

	[[nodiscard]] PlayerType GetTableRow() const
	{
		QFont font13("Segoe UI", 1, QFont::Bold);
		font13.setPixelSize(13);
		QFont font16("Segoe UI", 1, QFont::Bold);
		font16.setPixelSize(16);

		QColor bg = this->prColor.QColor();
		auto shipItem = this->ship ? this->ship->GetField(font13, bg, Qt::AlignVCenter | Qt::AlignLeft)  : new QTableWidgetItem();
		
		return {
				this->GetNameField(),
				shipItem,
				this->battles.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
				this->winrate.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
				this->avgDmg.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
				this->battlesShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
				this->winrateShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
				this->avgDmgShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight)
		};
	}

	[[nodiscard]] FieldType GetNameField() const
	{
		if (this->clan)
		{
			auto label = new QLabel();
			label->setTextFormat(Qt::RichText);
			label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
			label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
			label->setContentsMargins(3, 0, 3, 0);
			auto text = std::format(
					"<span style=\"color: {};\">[{}]</span>{}",
					this->clan->color.ToString(), this->clan->tag, this->name);
			label->setText(QString::fromStdString(text));

			if (this->prColor.Valid())
			{
				label->setAutoFillBackground(true);
				label->setStyleSheet(
						QString::fromStdString(std::format(
								"background-color: {}; font-size: 13px; font-family: Segoe UI;", this->prColor.ToString()
						)));
			}
			else
			{
				label->setStyleSheet(QString::fromStdString("font-size: 13px; font-family: Segoe UI;"));
			}

			return label;
		}
		else
		{
			auto label = new QTableWidgetItem(ToQString(this->name));
			QFont font("Segoe UI");
			font.setPixelSize(13);
			label->setFont(font);
			label->setTextAlignment(Qt::AlignVCenter);
			if (this->prColor.Valid())
				label->setBackground(this->prColor.QColor());
			return label;
		}
	}
};

[[maybe_unused]] static void from_json(const json& j, Player& p)
{
	if (!j.at("clan").is_null())
		p.clan = j.at("clan").get<Clan>();
	j.at("hidden_profile").get_to(p.hiddenPro);
	j.at("name").get_to(p.name);
	p.nameColor = Color(j.at("name_color").get<std::array<int, 3>>());
	if (!j.at("ship").is_null())
		p.ship = j.at("ship").get<Ship>();
	j.at("battles").get_to(p.battles);
	j.at("win_rate").get_to(p.winrate);
	j.at("avg_dmg").get_to(p.avgDmg);
	j.at("battles_ship").get_to(p.battlesShip);
	j.at("win_rate_ship").get_to(p.winrateShip);
	j.at("avg_dmg_ship").get_to(p.avgDmgShip);
	p.prColor = Color(j.at("pr_color").get<std::array<int, 4>>());
	j.at("wows_numbers_link").get_to(p.wowsNumbers);
}

struct Team
{
	uint8_t id;
	std::vector<Player> players;
	Stat avgDmg;
	Stat avgWr;
};

[[maybe_unused]] static void from_json(const json& j, Team& t)
{
	j.at("id").get_to(t.id);
	j.at("players").get_to(t.players);
	j.at("avg_dmg").get_to(t.avgDmg);
	j.at("avg_win_rate").get_to(t.avgWr);
}

struct Match
{
	Team team1;
	Team team2;
	std::string matchGroup;
	std::string statsMode;
	std::string region;
	std::string map;
	std::string dateTime;
	// std::string player;
	// std::string ship;
	// std::string shipIdent;
};

[[maybe_unused]] static void from_json(const json& j, Match& m)
{
	j.at("team1").get_to(m.team1);
	j.at("team2").get_to(m.team2);
	j.at("match_group").get_to(m.matchGroup);
	j.at("stats_mode").get_to(m.statsMode);
	j.at("region").get_to(m.region);
	j.at("map").get_to(m.map);
	j.at("date_time").get_to(m.dateTime);
	// j.at("player").get_to(m.player);
	// j.at("ship").get_to(m.ship);
	// j.at("ship_ident").get_to(m.shipIdent);
}

static std::string GetCSV(const Match& match)
{
	std::string out;
	out += "Team;Player;Clan;Ship;Matches;Winrate;AverageDamage;MatchesShip;WinrateShip;AverageDamageShip;WowsNumbers\n";

	auto getTeam = [&out](const Team& team, std::string_view teamID)
	{
		for (auto& player : team.players)
		{
			std::string clanName;
			if (player.clan)
				clanName = player.clan->name;

			std::string shipName;
			if (player.ship)
				shipName = player.ship->Name;

			out += std::format("{};{};{};{};{};{};{};{};{};{};{}\n",
					teamID, player.name, clanName, shipName,
					player.battles.string, player.winrate.string,
					player.avgDmg.string, player.battlesShip.string,
					player.winrateShip.string, player.avgDmgShip.string,
					player.wowsNumbers);
		}
	};
	getTeam(match.team1, "1");
	getTeam(match.team2, "2");

	return out;
}

}  // namespace _JSON


namespace pn = PotatoAlert::Client::StatsParser;

void pn::Label::UpdateLabel(QLabel* label) const
{
	label->setText(this->Text);

	if (this->Color)
	{
		auto palette = label->palette();
		palette.setColor(QPalette::WindowText, this->Color.value());
		label->setPalette(palette);
	}
}

StatsParseResult pn::ParseMatch(const json& j, const MatchContext& matchContext, bool parseCsv) noexcept
{
	StatsParseResult result;

	const auto match = j.get<_JSON::Match>();

	if (parseCsv)
		result.Csv = _JSON::GetCSV(match);

	_JSON::Ship playerShip;


	// parse match stats
	auto getTeam = [&match, &matchContext, &playerShip](const _JSON::Team& inTeam, Team& outTeam)
	{
		// do not display bots in scenario or operation mode
		if ((match.matchGroup == "pve" || match.matchGroup == "pve_premade") && inTeam.id == 2)
		{
			return;
		}

		TeamType teamTable;
		for (auto& player : inTeam.players)
		{
			if (player.name == matchContext.PlayerName && player.ship.has_value())
			{
				playerShip = player.ship.value();
			}

			auto row = player.GetTableRow();
			teamTable.push_back(row);
			outTeam.WowsNumbers.push_back(ToQString(player.wowsNumbers));
		}
		outTeam.AvgDmg = inTeam.avgDmg.GetLabel();
		outTeam.Winrate = inTeam.avgWr.GetLabel("%");
		outTeam.Table = teamTable;
	};
	getTeam(match.team1, result.Match.Team1);
	getTeam(match.team2, result.Match.Team2);

	// parse clan tag+name
	if (match.matchGroup == "clan")
	{
		auto findClan = [](const _JSON::Team& inTeam, Team& outTeam)
		{
			struct Clan
			{
				size_t count;
				const _JSON::Clan& clan;
			};
			std::map<std::string, Clan> clans;
			for (const _JSON::Player& player : inTeam.players)
			{
				if (!player.clan)
					continue;

				const _JSON::Clan& clan = player.clan.value();
				if (clans.contains(clan.tag))
				{
					clans.at(clan.tag).count++;
				}
				else
				{
					clans.emplace(clan.tag, Clan{ 1, clan });
				}
			}

			if (!clans.empty())
			{
				outTeam.Clan.Show = true;
				const auto max_elem = std::ranges::max_element(clans, [](const auto& a, const auto& b)
				{
					return a.second.count < b.second.count;
				});

				outTeam.Clan.Tag = max_elem->second.clan.GetTagLabel();
				outTeam.Clan.Name = max_elem->second.clan.GetNameLabel();
				outTeam.Clan.Region = max_elem->second.clan.GetRegionLabel();
			}
		};
		findClan(match.team1, result.Match.Team1);
		findClan(match.team2, result.Match.Team2);
	}

	// parse match info
	MatchType::InfoType& info = result.Match.Info;
	info.MatchGroup = std::move(match.matchGroup);
	info.StatsMode = std::move(match.statsMode);
	info.Region = std::move(match.region);
	info.Map = std::move(match.map);
	info.DateTime = std::move(match.dateTime);
	info.Player = std::move(matchContext.PlayerName);
	info.ShipIdent = std::move(matchContext.ShipIdent);
	info.ShipName = std::move(playerShip.Name);
	info.ShipClass = std::move(playerShip.Class);
	info.ShipNation = std::move(playerShip.Nation);
	info.ShipTier = playerShip.Tier;

	result.Success = true;
	return result;
}

StatsParseResult pn::ParseMatch(const std::string& raw, const MatchContext& matchContext, bool parseCsv) noexcept
{
	json j;
	sax_no_exception sax(j);
	if (!json::sax_parse(raw, &sax))
	{
		LOG_ERROR("ParseError while parsing server response JSON.");
		return StatsParseResult{};
	}

	return ParseMatch(j, matchContext, parseCsv);
}

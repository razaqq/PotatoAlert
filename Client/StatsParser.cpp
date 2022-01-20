// Copyright 2021 <github.com/razaqq>

#include "StatsParser.hpp"

#include "Core/Json.hpp"

#include <QLabel>

#include <array>
#include <format>
#include <optional>
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
	std::string_view string;
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
	std::string_view name;
	std::string_view tag;
	Color color;
	std::string_view region;

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
	std::string_view name;
	Color color;

	[[nodiscard]] QTableWidgetItem* GetField(const QFont& font, const QColor& bg, const QFlags<Qt::AlignmentFlag>& align) const
	{
		auto item = new QTableWidgetItem(ToQString(this->name));
		item->setForeground(this->color.QColor());
		item->setBackground(bg);
		item->setFont(font);
		item->setTextAlignment(align);
		return item;
	}
};

[[maybe_unused]] static void from_json(const json& j, Ship& s)
{
	j.at("name").get_to(s.name);
	s.color = Color(j.at("color").get<std::array<int, 3>>());
}

struct Player
{
	std::optional<Clan> clan;
	bool hiddenPro;
	std::string_view name;
	Color nameColor;
	std::optional<Ship> ship;
	Stat battles;
	Stat winrate;
	Stat avgDmg;
	Stat battlesShip;
	Stat winrateShip;
	Stat avgDmgShip;
	Color prColor;
	std::string_view wowsNumbers;

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
	j.at("hiddenPro").get_to(p.hiddenPro);
	j.at("name").get_to(p.name);
	p.nameColor = Color(j.at("nameColor").get<std::array<int, 3>>());
	if (!j.at("ship").is_null())
		p.ship = j.at("ship").get<Ship>();
	j.at("battles").get_to(p.battles);
	j.at("winrate").get_to(p.winrate);
	j.at("avgDmg").get_to(p.avgDmg);
	j.at("battlesShip").get_to(p.battlesShip);
	j.at("winrateShip").get_to(p.winrateShip);
	j.at("avgDmgShip").get_to(p.avgDmgShip);
	p.prColor = Color(j.at("prColor").get<std::array<int, 4>>());
	j.at("wowsNumbers").get_to(p.wowsNumbers);
}

struct Team
{
	size_t id;
	std::vector<Player> players;
	Stat avgDmg;
	Stat avgWr;
};

[[maybe_unused]] static void from_json(const json& j, Team& t)
{
	j.at("id").get_to(t.id);
	j.at("players").get_to(t.players);
	j.at("avgDmg").get_to(t.avgDmg);
	j.at("avgWr").get_to(t.avgWr);
}

struct Match
{
	Team team1;
	Team team2;
	std::string_view matchGroup;
	std::string_view statsMode;
	std::string_view region;
	std::string_view player;
	std::string_view ship;
	std::string_view map;
	std::string_view dateTime;
};

[[maybe_unused]] static void from_json(const json& j, Match& m)
{
	j.at("team1").get_to(m.team1);
	j.at("team2").get_to(m.team2);
	j.at("matchGroup").get_to(m.matchGroup);
	j.at("statsMode").get_to(m.statsMode);
	j.at("region").get_to(m.region);
	j.at("player").get_to(m.player);
	j.at("ship").get_to(m.ship);
	j.at("map").get_to(m.map);
	j.at("dateTime").get_to(m.dateTime);
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
				shipName = player.ship->name;

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
	label->setText(this->text);

	if (this->color)
	{
		auto palette = label->palette();
		palette.setColor(QPalette::WindowText, this->color.value());
		label->setPalette(palette);
	}
}

StatsParseResult pn::ParseMatch(const std::string& raw, bool parseCsv) noexcept
{
	StatsParseResult result;
	
	json j;
	sax_no_exception sax(j);
	if (!json::sax_parse(raw, &sax))
	{
		LOG_ERROR("ParseError while parsing server response JSON.");
		return result;
	}

	const auto match = j.get<_JSON::Match>();

	if (parseCsv)
		result.csv = _JSON::GetCSV(match);

	// parse match stats
	auto getTeam = [](const _JSON::Team& inTeam, Team& outTeam)
	{
		TeamType teamTable;
		for (auto& player : inTeam.players)
		{
			auto row = player.GetTableRow();
			teamTable.push_back(row);
			outTeam.wowsNumbers.push_back(ToQString(player.wowsNumbers));
		}
		outTeam.avgDmg = inTeam.avgDmg.GetLabel();
		outTeam.winrate = inTeam.avgWr.GetLabel("%");
		outTeam.table = teamTable;
	};
	getTeam(match.team1, result.match.team1);
	getTeam(match.team2, result.match.team2);

	// parse clan tag+name
	if (match.matchGroup == "clan")
	{
		auto findClan = [](const _JSON::Team& inTeam, Team& outTeam)
		{
			for (auto& player : inTeam.players)
			{
				if (!player.clan)
					continue;
				outTeam.clan.show = true;
				outTeam.clan.tag = player.clan->GetTagLabel();
				outTeam.clan.name = player.clan->GetNameLabel();
				outTeam.clan.region = player.clan->GetRegionLabel();
			}
		};
		findClan(match.team1, result.match.team1);
		findClan(match.team2, result.match.team2);
	}

	// parse match info
	result.match.info.matchGroup = match.matchGroup;
	result.match.info.statsMode = match.statsMode;
	result.match.info.map = match.map;
	result.match.info.ship = match.ship;
	result.match.info.dateTime = match.dateTime;
	result.match.info.region = match.region;
	result.match.info.player = match.player;

	result.success = true;
	return result;
}

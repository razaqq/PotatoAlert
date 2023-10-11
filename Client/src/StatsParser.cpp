// Copyright 2021 <github.com/razaqq>

#include "Client/StatsParser.hpp"

#include "Core/Format.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"

#include <QApplication>
#include <QGraphicsEffect>
#include <QHBoxLayout>
#include <QLabel>

#include <array>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#ifdef _MSC_VER
	#undef GetObject
#endif


using namespace PotatoAlert;
using namespace PotatoAlert::Client::StatsParser;
using PotatoAlert::Core::JsonResult;

namespace {

static QString ToQString(const std::string_view& str)
{
	return QString::fromUtf8(str.data(), static_cast<int>(str.size()));
}

}


namespace _JSON {

struct Color
{
	Color() = default;
	[[maybe_unused]] Color(int r, int g, int b) : m_r(r), m_g(g), m_b(b) {}
	[[maybe_unused]] Color(int r, int g, int b, int a) : m_r(r), m_g(g), m_b(b), m_a(a) {}
	[[maybe_unused]] explicit Color(const std::array<int, 3>& arr) : m_r(arr[0]), m_g(arr[1]), m_b(arr[2]) {}
	[[maybe_unused]] explicit Color(const std::array<int, 4>& arr) : m_r(arr[0]), m_g(arr[1]), m_b(arr[2]), m_a(arr[3]) {}

	[[nodiscard]] std::string ToString() const
	{
		if (!m_a)
			return fmt::format("rgb({}, {}, {})", m_r, m_g, m_b);
		else
			return fmt::format("rgba({}, {}, {}, {})", m_r, m_g, m_b, m_a.value());
	}

	[[nodiscard]] bool Valid() const
	{
		auto valid = [](int i) -> bool { return i >= 0 && i <= 255; };
		if (!m_a.has_value())
			return valid(m_r) && valid(m_g) && valid(m_b);
		else
			return valid(m_r) && valid(m_g) && valid(m_b) && valid(m_a.value());
	}

	[[nodiscard]] QColor GetQColor() const
	{
		if (!m_a)
			return QColor::fromRgb(m_r, m_g, m_b);
		else
			return QColor::fromRgb(m_r, m_g, m_b, m_a.value());
	}

private:
	int m_r, m_g, m_b;
	std::optional<int> m_a;
};

template<size_t Size>
static JsonResult<Color> ToColor(const rapidjson::Value& value, std::string_view key)
{
	if (!value.HasMember(key.data()))
		return PA_JSON_ERROR("Json color object has no key '{}'", key);

	std::array<int, Size> color = {};
	if (!Core::FromJson(value[key.data()], color))
	{
		return PA_JSON_ERROR("Failed to get stat color json as array");
	}
	return Color(color);
}

struct Stat
{
	std::string string;
	Color color;

	[[nodiscard]] QTableWidgetItem* GetField(const QFont& font, const QColor& bg, const QFlags<Qt::AlignmentFlag>& align) const
	{
		auto item = new QTableWidgetItem(ToQString(this->string));
		item->setForeground(this->color.GetQColor());
		item->setBackground(bg);
		item->setFont(font);
		item->setTextAlignment(align);
		return item;
	}

	[[nodiscard]] Label GetLabel(const QString& suffix = "") const
	{
		return {ToQString(this->string) + suffix, this->color.GetQColor()};
	}
};

static JsonResult<void> FromJson(const rapidjson::Value& j, std::string_view key, Stat& s)
{
	if (!j.HasMember(key.data()))
		return PA_JSON_ERROR("Json object for Stat has no key '{}'", key);

	PA_TRYA(s.string, Core::FromJson<std::string>(j[key.data()], "string"));
	PA_TRYA(s.color, ToColor<3>(j[key.data()], "color"));
	return {};
}

struct Clan
{
	std::string name;
	std::string tag;
	Color color;
	std::string region;

	[[nodiscard]] Label GetTagLabel() const
	{
		return { "[" + ToQString(this->tag) + "] ", this->color.GetQColor() };
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

static JsonResult<void> FromJson(const rapidjson::Value& j, Clan& c)
{
	PA_TRYA(c.name, Core::FromJson<std::string>(j, "name"));
	PA_TRYA(c.tag, Core::FromJson<std::string>(j, "tag"));
	PA_TRYA(c.color, ToColor<3>(j, "color"));
	PA_TRYA(c.region, Core::FromJson<std::string>(j, "region"));
	return {};
}

struct Ship
{
	std::string Name;
	std::string Class;
	std::string Nation;
	uint8_t Tier;
	// Color color;

	[[nodiscard]] QWidget* GetField(const QFont& font, const Color& bg, const QFlags<Qt::AlignmentFlag>& align) const
	{
		QWidget* ship = new QWidget();
		QHBoxLayout* layout = new QHBoxLayout();
		layout->setContentsMargins(3, 0, 3, 0);
		layout->setSpacing(3);

		QLabel* shipIcon = new QLabel();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		shipIcon->setPixmap(QIcon(fmt::format(":/{}.svg", Class).c_str()).pixmap(QSize(18, 9), qApp->devicePixelRatio()));
#else
		shipIcon->setPixmap(QIcon(fmt::format(":/{}.svg", Class).c_str()).pixmap(QSize(18, 9)));
#endif
		shipIcon->setStyleSheet("background-color: transparent;");
		shipIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		shipIcon->setAlignment(Qt::AlignLeft);
		// shipIcon->setFixedWidth(21);
		auto effect = new QGraphicsOpacityEffect();
		effect->setOpacity(0.85);
		shipIcon->setGraphicsEffect(effect);

		QLabel* shipTier = new QLabel();
		shipTier->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		shipTier->setStyleSheet("background-color: transparent;");
		if (Tier == 11)
		{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			shipTier->setPixmap(QIcon(":/Star.svg").pixmap(QSize(13, 13), qApp->devicePixelRatio()));
#else
			shipTier->setPixmap(QIcon(":/Star.svg").pixmap(QSize(13, 13)));
#endif
		}
		else
		{
			shipTier->setText(Client::TierToString(Tier).data());
			shipTier->setFont(font);
		}

		QLabel* shipName = new QLabel(Name.c_str());
		shipName->setFont(font);
		shipName->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
		shipName->setStyleSheet("background-color: transparent;");

		layout->addWidget(shipIcon, 0, align);
		layout->addWidget(shipTier, 0, align);
		layout->addWidget(shipName, 0, align);
		layout->addStretch();

		ship->setLayout(layout);
		ship->setStyleSheet(fmt::format("background-color: {};", bg.ToString()).c_str());

		return ship;
	}
};

static JsonResult<void> FromJson(const rapidjson::Value& j, Ship& s)
{
	PA_TRYA(s.Name, Core::FromJson<std::string>(j, "name"));
	PA_TRYA(s.Class, Core::FromJson<std::string>(j, "class"));
	PA_TRYA(s.Nation, Core::FromJson<std::string>(j, "nation"));
	PA_TRYA(s.Tier, Core::FromJson<uint8_t>(j, "tier"));
	return {};
}

struct Player
{
	std::optional<Clan> clan;
	bool hiddenPro;
	std::string Name;
	Color nameColor;
	std::optional<Ship> ship;
	Stat battles;
	Stat winrate;
	Stat avgDmg;
	Stat battlesShip;
	Stat winrateShip;
	Stat avgDmgShip;
	std::optional<Stat> Karma;
	Color PrColor;
	std::string wowsNumbers;
	bool IsUsingPa;

	[[nodiscard]] PlayerType GetTableRow(bool showKarma) const
	{
		QFont font13("Segoe UI", 1, QFont::Bold);
		font13.setPixelSize(13);
		QFont font16("Segoe UI", 1, QFont::Bold);
		font16.setPixelSize(16);

		const QColor bg = PrColor.GetQColor();
		QWidget* shipItem = ship ? ship->GetField(font13, PrColor, Qt::AlignVCenter | Qt::AlignLeft)  : new QWidget();
		
		PlayerType row =
		{
			GetNameField(showKarma),
			shipItem,
			battles.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
			winrate.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
			avgDmg.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
			battlesShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
			winrateShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight),
			avgDmgShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight)
		};
		return row;
	}

	[[nodiscard]] QWidget* GetNameField(bool showKarma) const
	{
		QWidget* name = new QWidget();
		QHBoxLayout* layout = new QHBoxLayout();
		layout->setContentsMargins(3, 0, 3, 0);
		layout->setSpacing(3);

		QLabel* nameLabel = new QLabel();
		nameLabel->setStyleSheet("background-color: transparent");
		if (clan)
		{
			nameLabel->setTextFormat(Qt::RichText);
			nameLabel->setText(fmt::format("<span style=\"color: {};\">[{}]</span>{}", clan->color.ToString(), clan->tag, Name).c_str());
		}
		else
		{
			nameLabel->setText(Name.c_str());
			QFont font("Segoe UI");
			font.setPixelSize(13);
			nameLabel->setFont(font);
		}
		layout->addWidget(nameLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

		if (Karma && showKarma)
		{
			QHBoxLayout* karmaLayout = new QHBoxLayout();
			karmaLayout->setSpacing(0);
			karmaLayout->setContentsMargins(0, 3, 0, 0);

			QLabel* karmaLabel = new QLabel();
			karmaLabel->setObjectName("karmaLabel");
			karmaLabel->setTextFormat(Qt::RichText);
			karmaLabel->setText(fmt::format("<span style=\"color: {};\">{}</span>", Karma.value().color.ToString(), Karma.value().string).c_str());
			karmaLabel->setStyleSheet("background-color: transparent");
			QFont font("Segoe UI");
			font.setPixelSize(11);
			karmaLabel->setFont(font);

			karmaLayout->addWidget(karmaLabel, 0, Qt::AlignTop | Qt::AlignLeft);
			layout->addLayout(karmaLayout, 0);
		}

		if (IsUsingPa)
		{
			QLabel* potatoIcon = new QLabel();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			potatoIcon->setPixmap(QIcon(":/potato.svg").pixmap(QSize(12, 12), qApp->devicePixelRatio()));
#else
			potatoIcon->setPixmap(QIcon(":/potato.svg").pixmap(QSize(12, 12)));
#endif
			potatoIcon->setStyleSheet("background-color: transparent;");
			potatoIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
			potatoIcon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

			layout->addWidget(potatoIcon, 0, Qt::AlignVCenter | Qt::AlignLeft);
		}

		layout->addStretch();
		name->setLayout(layout);
		if (PrColor.Valid())
		{
			name->setStyleSheet(fmt::format("background-color: {};", PrColor.ToString()).c_str());
		}

		return name;
	}
};

static JsonResult<void> FromJson(const rapidjson::Value& j, Player& p)
{
	if (j.HasMember("clan") && !j["clan"].IsNull())
	{
		Clan clan;
		PA_TRYV(FromJson(j["clan"], clan));
		p.clan = clan;
	}
	PA_TRYA(p.hiddenPro, Core::FromJson<bool>(j, "hidden_profile"));
	PA_TRYA(p.Name, Core::FromJson<std::string>(j, "name"));
	PA_TRYA(p.nameColor, ToColor<3>(j, "name_color"));
	if (j.HasMember("ship") && !j["ship"].IsNull())
	{
		Ship ship;
		PA_TRYV(FromJson(j["ship"], ship));
		p.ship = ship;
	}
	PA_TRYV(FromJson(j, "battles", p.battles));
	PA_TRYV(FromJson(j, "win_rate", p.winrate));
	PA_TRYV(FromJson(j, "avg_dmg", p.avgDmg));
	PA_TRYV(FromJson(j, "battles_ship", p.battlesShip));
	PA_TRYV(FromJson(j, "win_rate_ship", p.winrateShip));
	PA_TRYV(FromJson(j, "avg_dmg_ship", p.avgDmgShip));
	PA_TRYA(p.PrColor, ToColor<4>(j, "pr_color"));

	PA_TRYA(p.wowsNumbers, Core::FromJson<std::string>(j, "wows_numbers_link"));

	if (j.HasMember("karma"))
	{
		Stat karma;
		PA_TRYV(FromJson(j, "karma", karma));
		p.Karma = karma;
	}

	if (j.HasMember("is_using_pa"))
	{
		PA_TRYA(p.IsUsingPa, Core::FromJson<bool>(j, "is_using_pa"));
	}
	else
	{
		p.IsUsingPa = false;
	}
	return {};
}

struct Team
{
	uint8_t id;
	std::vector<Player> players;
	Stat avgDmg;
	Stat avgWr;
};

static JsonResult<void> FromJson(const rapidjson::Value& j, Team& t)
{
	PA_TRYV(Core::FromJson(j, "id", t.id));

	const auto playersValue = j["players"].GetArray();
	t.players.reserve(playersValue.Size());
	for (const auto& m : playersValue)
	{
		Player player;
		PA_TRYV(FromJson(m, player));
		t.players.emplace_back(player);
	}
	// PA_TRYV(Core::FromJson(j, "players", t.players));
	PA_TRYV(FromJson(j, "avg_dmg", t.avgDmg));
	PA_TRYV(FromJson(j, "avg_win_rate", t.avgWr));
	return {};
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
};

static JsonResult<void> FromJson(const rapidjson::Value& j, Match& m)
{
	PA_TRYV(FromJson(j["team1"], m.team1));
	PA_TRYV(FromJson(j["team2"], m.team2));
	PA_TRYA(m.matchGroup, Core::FromJson<std::string>(j, "match_group"));
	PA_TRYA(m.statsMode, Core::FromJson<std::string>(j, "stats_mode"));
	PA_TRYA(m.region, Core::FromJson<std::string>(j, "region"));
	PA_TRYA(m.map, Core::FromJson<std::string>(j, "map"));
	PA_TRYA(m.dateTime, Core::FromJson<std::string>(j, "date_time"));

	return {};
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

			out += fmt::format("{};{};{};{};{};{};{};{};{};{};{}\n",
					teamID, player.Name, clanName, shipName,
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
	label->setText(Text);

	if (Color)
	{
		auto palette = label->palette();
		palette.setColor(QPalette::WindowText, this->Color.value());
		label->setPalette(palette);
	}
}

JsonResult<StatsParseResult> pn::ParseMatch(const rapidjson::Value& j, const MatchContext& matchContext, bool showKarma) noexcept
{
	StatsParseResult result;

	_JSON::Match match;
	PA_TRYV(FromJson(j, match));

	result.Csv = GetCSV(match);

	_JSON::Ship playerShip;

	// parse match stats
	auto getTeam = [&match, &matchContext, &playerShip, showKarma](const _JSON::Team& inTeam, Team& outTeam)
	{
		// do not display bots in scenario or operation mode
		if ((match.matchGroup == "pve" || match.matchGroup == "pve_premade") && inTeam.id == 2)
		{
			return;
		}

		TeamType teamTable;
		for (auto& player : inTeam.players)
		{
			if (player.Name == matchContext.PlayerName && player.ship.has_value())
			{
				playerShip = player.ship.value();
			}

			teamTable.push_back(player.GetTableRow(showKarma));
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

	return result;
}

JsonResult<StatsParseResult> pn::ParseMatch(const std::string& raw, const MatchContext& matchContext, bool showKarma) noexcept
{
	PA_TRY(j, Core::ParseJson(raw));
	PA_TRY(match, ParseMatch(j, matchContext ,showKarma));
	return match;
}

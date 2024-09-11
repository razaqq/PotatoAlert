// Copyright 2021 <github.com/razaqq>

#include "Client/StatsParser.hpp"

#include "Core/Format.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Time.hpp"

#include <QApplication>
#include <QGraphicsEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

#include <array>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#ifdef _MSC_VER
	#undef GetObject
#endif


using namespace PotatoAlert;
using namespace PotatoAlert::Client::StatsParser;
using PotatoAlert::Core::JsonResult;

namespace {

static QString ToQString(std::string_view str)
{
	return QString::fromUtf8(str.data(), static_cast<int>(str.size()));
}

void AddShadow(QWidget* widget)
{
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(widget);
	shadow->setOffset(1, 1);
	shadow->setColor(QApplication::palette().base().color());
	widget->setGraphicsEffect(shadow);
}

}  // namespace


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
		auto valid = [](int i) -> bool
		{
			return i >= 0 && i <= 255;
		};
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

class ShadowLabel : public QLabel
{
public:
	using QLabel::QLabel;

	ShadowLabel(const QString& text, const QColor& fg, const QColor& bg, Qt::Alignment textAlignment, bool fontShadow) : m_fg(fg), m_bg(bg), m_alignment(textAlignment), m_fontShadow(fontShadow)
	{
		setText(text);
	}

private:
	void paintEvent(QPaintEvent* event) override
	{
		QPainter painter(this);

		painter.fillRect(contentsRect(), m_bg);

		const QRect textRect = contentsRect().adjusted(3, 3, -3, -3);

		if (m_fontShadow)
		{
			painter.setPen(palette().base().color());
			painter.drawText(textRect.adjusted(1, 1, 1, 1), m_alignment, text());
		}

		painter.setPen(m_fg);
		painter.drawText(textRect, m_alignment, text());
	}

private:
	QColor m_fg;
	QColor m_bg;
	Qt::Alignment m_alignment;
	bool m_fontShadow;
};

struct Stat
{
	std::string Str;
	Color ColorRGB;

	[[nodiscard]] ShadowLabel* GetField(const QFont& font, const QColor& bg, const QFlags<Qt::AlignmentFlag>& align, bool fontShadow) const
	{
		ShadowLabel* item = new ShadowLabel(ToQString(Str), ColorRGB.GetQColor(), bg, align, fontShadow);
		item->setFont(font);
		return item;
	}

	[[nodiscard]] Label GetLabel(const QString& suffix = "") const
	{
		return { ToQString(Str) + suffix, ColorRGB.GetQColor() };
	}
};

static JsonResult<void> FromJson(const rapidjson::Value& j, std::string_view key, Stat& s)
{
	if (!j.HasMember(key.data()))
		return PA_JSON_ERROR("Json object for Stat has no key '{}'", key);

	PA_TRYA(s.Str, Core::FromJson<std::string>(j[key.data()], "string"));
	PA_TRYA(s.ColorRGB, ToColor<3>(j[key.data()], "color"));
	return {};
}

struct Clan
{
	std::string Name;
	std::string Tag;
	Color ColorRGB;
	std::string Region;

	[[nodiscard]] Label GetTagLabel() const
	{
		return { "[" + ToQString(Tag) + "] ", ColorRGB.GetQColor() };
	}
	[[nodiscard]] Label GetNameLabel() const
	{
		return { ToQString(Name), std::nullopt };
	}
	[[nodiscard]] Label GetRegionLabel() const
	{
		return { ToQString(Region), std::nullopt };
	}
};

static JsonResult<void> FromJson(const rapidjson::Value& j, Clan& c)
{
	PA_TRYA(c.Name, Core::FromJson<std::string>(j, "name"));
	PA_TRYA(c.Tag, Core::FromJson<std::string>(j, "tag"));
	PA_TRYA(c.ColorRGB, ToColor<3>(j, "color"));
	PA_TRYA(c.Region, Core::FromJson<std::string>(j, "region"));
	return {};
}

struct Ship
{
	std::string Name;
	std::string Class;
	std::string Nation;
	uint8_t Tier;

	[[nodiscard]] QWidget* GetField(const QFont& font, const Color& bg, const QFlags<Qt::AlignmentFlag>& align, const MatchParseOptions& parseOptions) const
	{
		QWidget* ship = new QWidget();
		QHBoxLayout* layout = new QHBoxLayout();
		layout->setContentsMargins(3, 0, 3, 0);
		layout->setSpacing(3);

		QSize iconSize((int)std::roundf(18.0f * parseOptions.FontScaling), (int)std::roundf(9.0f * parseOptions.FontScaling));

		QLabel* shipIcon = new QLabel();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		shipIcon->setPixmap(QIcon(fmt::format(":/{}.svg", Class).c_str()).pixmap(iconSize, qApp->devicePixelRatio()));
#else
		shipIcon->setPixmap(QIcon(fmt::format(":/{}.svg", Class).c_str()).pixmap(iconSize));
#endif
		shipIcon->setStyleSheet("background-color: transparent;");
		shipIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		shipIcon->setAlignment(Qt::AlignLeft);
		QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
		effect->setOpacity(0.85);
		shipIcon->setGraphicsEffect(effect);

		if (parseOptions.FontShadow)
			AddShadow(shipIcon);

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

		if (parseOptions.FontShadow)
			AddShadow(shipTier);

		QLabel* shipName = new QLabel(Name.c_str());
		shipName->setFont(font);
		shipName->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
		shipName->setStyleSheet("background-color: transparent;");

		if (parseOptions.FontShadow)
			AddShadow(shipName);

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
	std::optional<Clan> Clan;
	bool HiddenPro;
	std::string Name;
	Color NameColor;
	std::optional<Ship> Ship;
	Stat Battles;
	Stat Winrate;
	Stat AvgDmg;
	Stat BattlesShip;
	Stat WinrateShip;
	Stat AvgDmgShip;
	std::optional<Stat> Karma;
	Color PrColor;
	std::string WowsNumbers;
	bool IsUsingPa;

	[[nodiscard]] PlayerType GetTableRow(const MatchParseOptions& parseOptions) const
	{
		QFont font13(QApplication::font().family(), 1, QFont::Bold);
		font13.setPixelSize((int)std::roundf(13.0f * parseOptions.FontScaling));
		QFont font16(QApplication::font().family(), 1, QFont::Bold);
		font16.setPixelSize((int)std::roundf(16.0f * parseOptions.FontScaling));

		const QColor bg = PrColor.GetQColor();
		QWidget* shipItem = Ship ? Ship->GetField(font13, PrColor, Qt::AlignVCenter | Qt::AlignLeft, parseOptions) : new QWidget();

		PlayerType row = {
			GetNameField(parseOptions),
			shipItem,
			Battles.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight, parseOptions.FontShadow),
			Winrate.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight, parseOptions.FontShadow),
			AvgDmg.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight, parseOptions.FontShadow),
			BattlesShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight, parseOptions.FontShadow),
			WinrateShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight, parseOptions.FontShadow),
			AvgDmgShip.GetField(font16, bg, Qt::AlignVCenter | Qt::AlignRight, parseOptions.FontShadow)
		};
		return row;
	}

	[[nodiscard]] QWidget* GetNameField(const MatchParseOptions& parseOptions) const
	{
		QWidget* name = new QWidget();
		QHBoxLayout* layout = new QHBoxLayout();
		layout->setContentsMargins(3, 0, 3, 0);
		layout->setSpacing(3);

		QLabel* nameLabel = new QLabel();
		nameLabel->setStyleSheet("background-color: transparent");
		const int nameSize = (int)std::roundf(13.0f * parseOptions.FontScaling);
		if (Clan)
		{
			nameLabel->setTextFormat(Qt::RichText);
			nameLabel->setText(fmt::format(R"(<span style="font-size: {}px;"><span style="color: {};">[{}]</span>{}</span>)", nameSize, Clan->ColorRGB.ToString(), Clan->Tag, Name).c_str());
		}
		else
		{
			nameLabel->setText(Name.c_str());
			QFont font(QApplication::font().family());
			font.setPixelSize(nameSize);
			nameLabel->setFont(font);
		}
		layout->addWidget(nameLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

		if (parseOptions.FontShadow)
			AddShadow(nameLabel);

		if (Karma && parseOptions.ShowKarma && !HiddenPro)
		{
			QHBoxLayout* karmaLayout = new QHBoxLayout();
			karmaLayout->setSpacing(0);
			karmaLayout->setContentsMargins(0, 3, 0, 0);

			QLabel* karmaLabel = new QLabel();
			karmaLabel->setObjectName("karmaLabel");
			karmaLabel->setTextFormat(Qt::RichText);
			karmaLabel->setText(fmt::format(R"(<span style="color: {};">{}</span>)", Karma.value().ColorRGB.ToString(), Karma.value().Str).c_str());
			karmaLabel->setStyleSheet("background-color: transparent");
			QFont font(QApplication::font().family());
			font.setPixelSize((int)std::roundf(11.0f * parseOptions.FontScaling));
			karmaLabel->setFont(font);

			if (parseOptions.FontShadow)
				AddShadow(karmaLabel);

			karmaLayout->addWidget(karmaLabel, 0, Qt::AlignTop | Qt::AlignLeft);
			layout->addLayout(karmaLayout, 0);
		}

		if (IsUsingPa)
		{
			const int potatoSize = (int)std::roundf(12.0f * parseOptions.FontScaling);
			QLabel* potatoIcon = new QLabel();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			potatoIcon->setPixmap(QIcon(":/potato.svg").pixmap(QSize(potatoSize, potatoSize), qApp->devicePixelRatio()));
#else
			potatoIcon->setPixmap(QIcon(":/potato.svg").pixmap(QSize(potatoSize, potatoSize)));
#endif
			potatoIcon->setStyleSheet("background-color: transparent;");
			potatoIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
			potatoIcon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

			if (parseOptions.FontShadow)
				AddShadow(potatoIcon);

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
		p.Clan = clan;
	}
	PA_TRYA(p.HiddenPro, Core::FromJson<bool>(j, "hidden_profile"));
	PA_TRYA(p.Name, Core::FromJson<std::string>(j, "name"));
	PA_TRYA(p.NameColor, ToColor<3>(j, "name_color"));
	if (j.HasMember("ship") && !j["ship"].IsNull())
	{
		Ship ship;
		PA_TRYV(FromJson(j["ship"], ship));
		p.Ship = ship;
	}
	PA_TRYV(FromJson(j, "battles", p.Battles));
	PA_TRYV(FromJson(j, "win_rate", p.Winrate));
	PA_TRYV(FromJson(j, "avg_dmg", p.AvgDmg));
	PA_TRYV(FromJson(j, "battles_ship", p.BattlesShip));
	PA_TRYV(FromJson(j, "win_rate_ship", p.WinrateShip));
	PA_TRYV(FromJson(j, "avg_dmg_ship", p.AvgDmgShip));
	PA_TRYA(p.PrColor, ToColor<4>(j, "pr_color"));

	PA_TRYA(p.WowsNumbers, Core::FromJson<std::string>(j, "wows_numbers_link"));

	if (j.HasMember("karma") && !j["karma"].IsNull())
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
	uint8_t Id;
	std::vector<Player> Players;
	Stat AvgDmg;
	Stat AvgWr;
};

static JsonResult<void> FromJson(const rapidjson::Value& j, Team& t)
{
	PA_TRYV(Core::FromJson(j, "id", t.Id));

	const auto playersValue = j["players"].GetArray();
	t.Players.reserve(playersValue.Size());
	for (const auto& m : playersValue)
	{
		Player player;
		PA_TRYV(FromJson(m, player));
		t.Players.emplace_back(player);
	}
	// PA_TRYV(Core::FromJson(j, "players", t.players));
	PA_TRYV(FromJson(j, "avg_dmg", t.AvgDmg));
	PA_TRYV(FromJson(j, "avg_win_rate", t.AvgWr));
	return {};
}

struct Match
{
	Team Team1;
	Team Team2;
	std::string MatchGroup;
	std::string StatsMode;
	std::string Region;
	std::string Map;
	std::string DateTime;
};

static JsonResult<void> FromJson(const rapidjson::Value& j, Match& m)
{
	PA_TRYV(FromJson(j["team1"], m.Team1));
	PA_TRYV(FromJson(j["team2"], m.Team2));
	PA_TRYA(m.MatchGroup, Core::FromJson<std::string>(j, "match_group"));
	PA_TRYA(m.StatsMode, Core::FromJson<std::string>(j, "stats_mode"));
	PA_TRYA(m.Region, Core::FromJson<std::string>(j, "region"));
	PA_TRYA(m.Map, Core::FromJson<std::string>(j, "map"));
	PA_TRYA(m.DateTime, Core::FromJson<std::string>(j, "date_time"));

	return {};
}

static std::string GetCSV(const Match& match)
{
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

}  // namespace _JSON

namespace pn = PotatoAlert::Client::StatsParser;

void pn::Label::UpdateLabel(QLabel* label) const
{
	label->setText(Text);

	if (Color)
	{
		auto palette = label->palette();
		palette.setColor(QPalette::WindowText, Color.value());
		label->setPalette(palette);
	}
}

JsonResult<StatsParseResult> pn::ParseMatch(const rapidjson::Value& j, const MatchContext& matchContext, MatchParseOptions&& parseOptions) noexcept
{
	StatsParseResult result;

	_JSON::Match match;
	PA_TRYV(FromJson(j, match));

	result.Csv = GetCSV(match);

	_JSON::Ship playerShip;

	// parse match stats
	auto getTeam = [&match, &matchContext, &playerShip, parseOptions](const _JSON::Team& inTeam, Team& outTeam)
	{
		// do not display bots in scenario or operation mode
		if ((match.MatchGroup == "pve" || match.MatchGroup == "pve_premade") && inTeam.Id == 2)
		{
			return;
		}

		TeamType teamTable;
		for (const _JSON::Player& player : inTeam.Players)
		{
			if (player.Name == matchContext.PlayerName && player.Ship.has_value())
			{
				playerShip = player.Ship.value();
			}

			teamTable.push_back(player.GetTableRow(parseOptions));
			outTeam.WowsNumbers.push_back(ToQString(player.WowsNumbers));
		}
		outTeam.AvgDmg = inTeam.AvgDmg.GetLabel();
		outTeam.Winrate = inTeam.AvgWr.GetLabel("%");
		outTeam.Table = teamTable;
	};
	getTeam(match.Team1, result.Match.Team1);
	getTeam(match.Team2, result.Match.Team2);

	// parse clan tag+name
	if (match.MatchGroup == "clan")
	{
		auto findClan = [](const _JSON::Team& inTeam, Team& outTeam)
		{
			struct Clan
			{
				size_t count;
				const _JSON::Clan& clan;
			};
			std::unordered_map<std::string, Clan> clans;
			for (const _JSON::Player& player : inTeam.Players)
			{
				if (!player.Clan)
					continue;

				const _JSON::Clan& clan = player.Clan.value();
				if (clans.contains(clan.Tag))
				{
					clans.at(clan.Tag).count++;
				}
				else
				{
					clans.emplace(clan.Tag, Clan{ 1, clan });
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
		findClan(match.Team1, result.Match.Team1);
		findClan(match.Team2, result.Match.Team2);
	}

	// parse match info
	MatchType::InfoType& info = result.Match.Info;
	info.MatchGroup = std::move(match.MatchGroup);
	info.StatsMode = std::move(match.StatsMode);
	info.Region = std::move(match.Region);
	info.Map = std::move(match.Map);

	// parse and convert dateTime
	if (const std::optional<Core::Time::TimePoint> tp = Core::Time::StrToTime(match.DateTime, "%d.%m.%Y %H:%M:%S"))
	{
		match.DateTime = Core::Time::TimeToStr(*tp, "{:%Y-%m-%d %H:%M:%S}");
	}

	info.DateTime = std::move(match.DateTime);
	info.Player = matchContext.PlayerName;
	info.ShipIdent = matchContext.ShipIdent;
	info.ShipName = std::move(playerShip.Name);
	info.ShipClass = std::move(playerShip.Class);
	info.ShipNation = std::move(playerShip.Nation);
	info.ShipTier = playerShip.Tier;

	return result;
}

JsonResult<StatsParseResult> pn::ParseMatch(const std::string& raw, const MatchContext& matchContext, MatchParseOptions&& parseOptions) noexcept
{
	PA_TRY(j, Core::ParseJson(raw));
	PA_TRY(match, ParseMatch(j, matchContext, std::move(parseOptions)));
	return match;
}

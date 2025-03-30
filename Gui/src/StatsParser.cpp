// Copyright 2025 <github.com/razaqq>

#include "Client/StatsParser.hpp"

#include "Core/Json.hpp"
#include "Core/Format.hpp"
#include "Core/Time.hpp"

#include "Gui/StatsParser.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QApplication>
#include <QColor>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QRect>

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>


using PotatoAlert::Client::MatchContext;
using PotatoAlert::Client::StatsParser::ColorRGB;
using PotatoAlert::Client::StatsParser::ColorRGBA;
using PotatoAlert::Core::JsonResult;
using PotatoAlert::Gui::StatsParser::FieldType;
using PotatoAlert::Gui::StatsParser::PlayerType;
using PotatoAlert::Gui::StatsParser::Match;
using PotatoAlert::Gui::StatsParser::MatchParseOptions;
using PotatoAlert::Gui::StatsParser::Label;
using PotatoAlert::ReplayParser::TierToString;
namespace pn = PotatoAlert::Gui::StatsParser;
namespace ClientStats = PotatoAlert::Client::StatsParser;

namespace {

class ShadowLabel : public QLabel
{
public:
	using QLabel::QLabel;

	explicit ShadowLabel(const QString& text, const QColor& fg, const QColor& bg, Qt::Alignment textAlignment, bool fontShadow) : m_fg(fg), m_bg(bg), m_alignment(textAlignment), m_fontShadow(fontShadow)
	{
		setText(text);
	}

private:
	void paintEvent([[maybe_unused]] QPaintEvent* event) override
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

static void AddShadow(QWidget* widget)
{
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(widget);
	shadow->setOffset(1, 1);
	shadow->setColor(QApplication::palette().base().color());
	widget->setGraphicsEffect(shadow);
}

static QString ToQString(std::string_view str)
{
	return QString::fromUtf8(str.data(), static_cast<qsizetype>(str.size()));
}

static QColor ToQColor(ColorRGB color)
{
	return QColor::fromRgb(color[0], color[1], color[2]);
}

static QColor ToQColor(ColorRGBA color)
{
	return QColor::fromRgb(color[0], color[1], color[2], color[3]);
}

static std::string ToString(ColorRGB color)
{
	return fmt::format("rgb({}, {}, {})", color[0], color[1], color[2]);
}

static std::string ToString(ColorRGBA color)
{
	return fmt::format("rgba({}, {}, {}, {})", color[0], color[1], color[2], color[3]);
}

static ShadowLabel* StatToField(const ClientStats::Stat& stat, const QFont& font, const QColor& bg, const QFlags<Qt::AlignmentFlag>& align, bool fontShadow)
{
	ShadowLabel* item = new ShadowLabel(ToQString(stat.Str), ToQColor(stat.ColorRGB), bg, align, fontShadow);
	item->setFont(font);
	return item;
}

static Label StatToLabel(const ClientStats::Stat& stat, const QString& suffix = "")
{
	return { ToQString(stat.Str) + suffix, ToQColor(stat.ColorRGB) };
}

static QWidget* ShipToField(const ClientStats::Ship& ship, const QFont& font, const ColorRGBA& bg, const QFlags<Qt::AlignmentFlag>& align, const MatchParseOptions& parseOptions)
{
	QWidget* shipWidget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(3, 0, 3, 0);
	layout->setSpacing(3);

	QSize iconSize((int)std::roundf(18.0f * parseOptions.FontScaling), (int)std::roundf(9.0f * parseOptions.FontScaling));

	QLabel* shipIcon = new QLabel();
	shipIcon->setPixmap(QIcon(fmt::format(":/{}.svg", ship.Class).c_str()).pixmap(iconSize, qApp->devicePixelRatio()));
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
	if (ship.Tier == 11)
	{
		shipTier->setPixmap(QIcon(":/Star.svg").pixmap(QSize(13, 13), qApp->devicePixelRatio()));
	}
	else
	{
		shipTier->setText(TierToString(ship.Tier).data());
		shipTier->setFont(font);
	}

	if (parseOptions.FontShadow)
		AddShadow(shipTier);

	QLabel* shipName = new QLabel(ship.Name.c_str());
	shipName->setFont(font);
	shipName->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	shipName->setStyleSheet("background-color: transparent;");

	if (parseOptions.FontShadow)
		AddShadow(shipName);

	layout->addWidget(shipIcon, 0, align);
	layout->addWidget(shipTier, 0, align);
	layout->addWidget(shipName, 0, align);
	layout->addStretch();

	shipWidget->setLayout(layout);
	shipWidget->setStyleSheet(fmt::format("background-color: {};", ToString(bg)).c_str());

	return shipWidget;
}

static QWidget* GetPlayerNameField(const ClientStats::Player& player, const MatchParseOptions& parseOptions)
{
	QWidget* name = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(3, 0, 3, 0);
	layout->setSpacing(3);

	QLabel* nameLabel = new QLabel();
	nameLabel->setStyleSheet("background-color: transparent");
	const int nameSize = (int)std::roundf(13.0f * parseOptions.FontScaling);
	if (player.Clan)
	{
		nameLabel->setTextFormat(Qt::RichText);
		nameLabel->setText(fmt::format(R"(<span style="font-size: {}px;"><span style="color: {};">[{}]</span>{}</span>)", nameSize, ToString(player.Clan->ColorRGB), player.Clan->Tag, player.Name).c_str());
	}
	else
	{
		nameLabel->setText(player.Name.c_str());
		QFont font(QApplication::font().family());
		font.setPixelSize(nameSize);
		nameLabel->setFont(font);
	}
	layout->addWidget(nameLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	if (parseOptions.FontShadow)
		AddShadow(nameLabel);

	if (player.Karma && parseOptions.ShowKarma && !player.HiddenPro)
	{
		QHBoxLayout* karmaLayout = new QHBoxLayout();
		karmaLayout->setSpacing(0);
		karmaLayout->setContentsMargins(0, 3, 0, 0);

		QLabel* karmaLabel = new QLabel();
		karmaLabel->setObjectName("karmaLabel");
		karmaLabel->setTextFormat(Qt::RichText);
		karmaLabel->setText(fmt::format(R"(<span style="color: {};">{}</span>)", ToString(player.Karma.value().ColorRGB), player.Karma.value().Str).c_str());
		karmaLabel->setStyleSheet("background-color: transparent");
		QFont font(QApplication::font().family());
		font.setPixelSize((int)std::roundf(11.0f * parseOptions.FontScaling));
		karmaLabel->setFont(font);

		if (parseOptions.FontShadow)
			AddShadow(karmaLabel);

		karmaLayout->addWidget(karmaLabel, 0, Qt::AlignTop | Qt::AlignLeft);
		layout->addLayout(karmaLayout, 0);
	}

	if (player.IsUsingPa)
	{
		const int potatoSize = (int)std::roundf(12.0f * parseOptions.FontScaling);
		QLabel* potatoIcon = new QLabel();
		potatoIcon->setPixmap(QIcon(":/potato.svg").pixmap(QSize(potatoSize, potatoSize), qApp->devicePixelRatio()));
		potatoIcon->setStyleSheet("background-color: transparent;");
		potatoIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		potatoIcon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

		if (parseOptions.FontShadow)
			AddShadow(potatoIcon);

		layout->addWidget(potatoIcon, 0, Qt::AlignVCenter | Qt::AlignLeft);
	}

	layout->addStretch();
	name->setLayout(layout);
	name->setStyleSheet(fmt::format("background-color: {};", ToString(player.PrColor)).c_str());

	return name;
}

static PlayerType GetTableRow(const ClientStats::Player& player, const MatchParseOptions& options)
{
	QFont font13(QApplication::font().family(), 1, QFont::Bold);
	font13.setPixelSize((int)std::roundf(13.0f * options.FontScaling));
	QFont font16(QApplication::font().family(), 1, QFont::Bold);
	font16.setPixelSize((int)std::roundf(16.0f * options.FontScaling));

	const QColor bg = ToQColor(player.PrColor);
	QWidget* shipItem = player.Ship ? ShipToField(*player.Ship, font13, player.PrColor, Qt::AlignVCenter | Qt::AlignLeft, options) : new QWidget();

	PlayerType row = {
		GetPlayerNameField(player, options),
		shipItem,
		StatToField(player.Battles, font16, bg, Qt::AlignVCenter | Qt::AlignRight, options.FontShadow),
		StatToField(player.Winrate, font16, bg, Qt::AlignVCenter | Qt::AlignRight, options.FontShadow),
		StatToField(player.AvgDmg, font16, bg, Qt::AlignVCenter | Qt::AlignRight, options.FontShadow),
		StatToField(player.BattlesShip, font16, bg, Qt::AlignVCenter | Qt::AlignRight, options.FontShadow),
		StatToField(player.WinrateShip, font16, bg, Qt::AlignVCenter | Qt::AlignRight, options.FontShadow),
		StatToField(player.AvgDmgShip, font16, bg, Qt::AlignVCenter | Qt::AlignRight, options.FontShadow)
	};
	return row;
}

static Label GetClanTagLabel(const ClientStats::Clan& clan)
{
	return { "[" + ToQString(clan.Tag) + "] ", ToQColor(clan.ColorRGB) };
}
static Label GetClanNameLabel(const ClientStats::Clan& clan)
{
	return { ToQString(clan.Name), std::nullopt };
}
static Label GetClanRegionLabel(const ClientStats::Clan& clan)
{
	return { ToQString(clan.Region), std::nullopt };
}

}  // namespace

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

Match pn::ParseMatch(const ClientStats::Match& match, const MatchContext& ctx, MatchParseOptions&& parseOptions) noexcept
{
	Match outMatch;
	ClientStats::Ship playerShip;

	// parse match stats
	auto getTeam = [&match, &ctx, &playerShip, parseOptions](const ClientStats::Team& inTeam, Team& outTeam)
	{
		// do not display bots in scenario or operation mode
		if ((match.MatchGroup == "pve" || match.MatchGroup == "pve_premade") && inTeam.Id == 2)
		{
			return;
		}

		TeamType teamTable;
		for (const ClientStats::Player& player : inTeam.Players)
		{
			if (player.Name == ctx.PlayerName && player.Ship.has_value())
			{
				playerShip = player.Ship.value();
			}

			teamTable.push_back(GetTableRow(player, parseOptions));
			outTeam.WowsNumbers.push_back(ToQString(player.WowsNumbers));
		}
		outTeam.AvgDmg = StatToLabel(inTeam.AvgDmg);
		outTeam.Winrate = StatToLabel(inTeam.AvgWr, "%");
		outTeam.Table = teamTable;
	};
	getTeam(match.Team1, outMatch.Team1);
	getTeam(match.Team2, outMatch.Team2);

	// parse clan tag+name
	if (match.MatchGroup == "clan")
	{
		auto findClan = [](const ClientStats::Team& inTeam, Team& outTeam)
		{
			struct Clan
			{
				size_t count;
				const ClientStats::Clan& clan;
			};
			std::unordered_map<std::string, Clan> clans;
			for (const ClientStats::Player& player : inTeam.Players)
			{
				if (!player.Clan)
					continue;

				const ClientStats::Clan& clan = player.Clan.value();
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
				const auto maxElem = std::ranges::max_element(clans, [](const auto& a, const auto& b)
				{
					return a.second.count < b.second.count;
				});

				outTeam.Clan.Tag = GetClanTagLabel(maxElem->second.clan);
				outTeam.Clan.Name = GetClanNameLabel(maxElem->second.clan);
				outTeam.Clan.Region = GetClanRegionLabel(maxElem->second.clan);
			}
		};
		findClan(match.Team1, outMatch.Team1);
		findClan(match.Team2, outMatch.Team2);
	}

	return outMatch;
}

JsonResult<Match> pn::ParseMatch(std::string_view json, const MatchContext& ctx, MatchParseOptions&& parseOptions) noexcept
{
	PA_TRY(match, ClientStats::ParseMatch(json));
	return ParseMatch(match, ctx, std::move(parseOptions));
}

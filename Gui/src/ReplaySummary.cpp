// Copyright 2022 <github.com/razaqq>

#include "Core/Format.hpp"

#include "Client/Config.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"
#include "Client/StringTable.hpp"

#include "Gui/Events.hpp"
#include "Gui/IconButton.hpp"
#include "Gui/ReplaySummary.hpp"

#include <QApplication>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QtWidgets/private/qpixmapfilter_p.h>

#include <string>


using namespace PotatoAlert::Client::StringTable;
using PotatoAlert::ReplayParser::AchievementType;
using PotatoAlert::ReplayParser::RibbonType;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::Match;
using PotatoAlert::Gui::Background;
using PotatoAlert::Gui::ShadowLabel;
using ReplaySummaryData = PotatoAlert::ReplayParser::ReplaySummary;
using ReplaySummaryGui = PotatoAlert::Gui::ReplaySummary;

namespace {

static std::string InsertEvery(std::string_view str, size_t every, char toInsert)
{
	size_t size = str.size() + (str.size() / every);
	if (str.size() % every == 0) size--;

	std::string out;
	out.reserve(size);

	for (size_t i = 0; i < str.size(); i++)
	{
		if ((str.size() - i) % 3 == 0 && !(i == 0 && str.size() % 3 == 0))
		{
			out.push_back(toInsert);
		}
		out.push_back(str[i]);
	}

	return out;
}

static std::string FormatDamageNumber(float dmg)
{
	std::string s = std::to_string(static_cast<int>(std::round(dmg)));
	return InsertEvery(s, 3, ' ');
}

static void ClearLayout(QLayout* layout)
{
	if (layout)
	{
		while (QLayoutItem* item = layout->takeAt(0))
		{
			if (QLayout* subLayout = item->layout())
			{
				ClearLayout(subLayout);
			}
			else if (QWidget* widget = item->widget())
			{
				delete widget;
			}
			delete item;
		}
		delete layout;
	}
}

static constexpr std::string_view GetMatchGroupName(std::string_view matchGroup)
{
	if (matchGroup == "pvp")
	{

	}
	return matchGroup;
}

static constexpr std::string_view GetRibbonName(int lang, RibbonType ribbon)
{
	switch (GetParent(ribbon))
	{
		case RibbonType::Artillery:
		case RibbonType::AcousticHit:
		case RibbonType::TorpedoHit:
		case RibbonType::SecondaryHit:
		case RibbonType::Bomb:
		case RibbonType::Rocket:
		case RibbonType::DepthChargeHit:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_HITS);
		case RibbonType::PlaneShotDown:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_PLANESHOWDOWN);
		case RibbonType::Incapacitation:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_INCAPACITATION);
		case RibbonType::Destroyed:
		case RibbonType::BuildingDestroyed:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_DESTROYED);
		case RibbonType::SetFire:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_SETONFIRE);
		case RibbonType::Flooding:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_FLOODING);
		case RibbonType::Citadel:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_CITADEL);
		case RibbonType::Defended:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_DEFENDED);
		case RibbonType::Captured:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_CAPTURED);
		case RibbonType::AssistedInCapture:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_ASSISTEDINCAPTURE);
		case RibbonType::Suppressed:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_SUPPRESSED);
		case RibbonType::Spotted:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_SPOTTED);
		case RibbonType::ShotDownByAircraft:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_SHOTDOWNBYAIRCRAFT);
		case RibbonType::BuffSeized:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_BUFFSEIZED);
		default:
			return GetStringView(lang, StringTableKey::REPLAY_RIBBON_UNKNOWN);
	}
}

}  // namespace

class Background : public QWidget
{
public:
	explicit Background(const char* map, QWidget* parent = nullptr)
		: QWidget(parent), m_map(map) {}  // m_nationFlag(nationFlag) {}

	void SetImage(const QString& map)  // , const QString& nationFlag)
	{
		m_map = map;
		// m_nationFlag = nationFlag;
		update();
		repaint();
	}

protected:
	void paintEvent(QPaintEvent* event) override
	{
		QPainter painter(this);
		QPixmap map = QPixmap::fromImage(QImage(m_map));

		QPixmapBlurFilter f;
		f.setRadius(4);
		f.setBlurHints(QGraphicsBlurEffect::QualityHint);
		f.draw(&painter, { 0, 0 }, map);

		// QPixmap flag = QPixmap::fromImage(QImage(m_nationFlag));
		// painter.drawPixmap(0, 0, map.scaledToHeight(height(), Qt::SmoothTransformation));
		// painter.drawPixmap(0, 0, flag.scaledToHeight(height(), Qt::SmoothTransformation));
		QWidget::paintEvent(event);
	}

private:
	QString m_map;
	// QString m_nationFlag;
	static constexpr float m_aspectRatio = 16.0f / 9.0f;
};

class ShadowLabel : public QLabel
{
public:
	explicit ShadowLabel(std::string_view text, int shadowX = 0, int shadowY = 0, int blurRadius = 0,
						 QColor shadowColor = QColor(0, 0, 0), QWidget* parent = nullptr) : QLabel(text.data(), parent)
	{
		QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
		effect->setBlurRadius(blurRadius);
		effect->setColor(shadowColor);
		effect->setOffset(shadowX, shadowY);
		setGraphicsEffect(effect);
		setObjectName("ReplaySummary_ShadowLabel");
	}
};

class Ribbon : public QWidget
{
public:
	Ribbon(QWidget* content, std::string_view title, std::string_view count = "", QWidget* parent = nullptr) : QWidget(parent)
	{
		setObjectName("ReplaySummary_Ribbon");
		QVBoxLayout* layout = new QVBoxLayout();
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		layout->addWidget(content, 0, Qt::AlignHCenter | Qt::AlignTop);

		m_titleLabel = new ShadowLabel(title.data(), 1, 1, 0, QColor(0, 0, 0, 242));
		m_titleLabel->setObjectName("ReplaySummary_RibbonTitle");

		layout->addWidget(m_titleLabel, 0, Qt::AlignHCenter | Qt::AlignTop);

		setLayout(layout);
	}

protected:
	void paintEvent(QPaintEvent* _) override
	{
		// support styling on custom QWidget
		QStyleOption opt;
		opt.initFrom(this);
		QPainter p(this);
		style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	}

private:
	friend class RibbonSmall;
	ShadowLabel* m_titleLabel;
};

class RibbonSmall : public Ribbon
{
public:
	RibbonSmall(QWidget* content, std::string_view title, std::string_view count = "", QWidget* parent = nullptr) : Ribbon(content, title, count, parent)
	{
		setObjectName("ReplaySummary_RibbonSmall");
		m_titleLabel->setObjectName("ReplaySummary_RibbonTitleSmall");
	}
};

void ReplaySummaryGui::paintEvent(QPaintEvent* _)
{
	// support styling on custom QWidget
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

ReplaySummaryGui::ReplaySummary(const Client::ServiceProvider& serviceProvider, QWidget* parent) : QWidget(parent), m_services(serviceProvider)
{
	QFontDatabase::addApplicationFont(":/Warhelios-Regular.ttf");
	QFontDatabase::addApplicationFont(":/Warhelios-Bold.ttf");
}

void ReplaySummaryGui::SetReplaySummary(const Match& match)
{
	ClearLayout(layout());

	const int lang = m_services.Get<Config>().Get<ConfigKey::Language>();

	m_background = new Background(
		fmt::format(":/{}.jpg", match.Map).c_str()
		// fmt::format(":/{}.png", match.ShipNation).c_str()
	);
	m_arWidget = new AspectRatioWidget(nullptr, m_background, 9.0f / 16.0f);

	// layout for the main widget
	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->setContentsMargins(10, 10, 10, 10);
	mainLayout->setSpacing(0);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_arWidget->setMinimumSize(800, static_cast<int>(800.0f * 9.0f / 16.0f));
	m_arWidget->setMaximumSize(1280, 720);
	m_arWidget->SetAspectWidget(m_background, 9.0f / 16.0f);

	mainLayout->addWidget(m_arWidget);
	mainLayout->setAlignment(Qt::AlignCenter);

	setLayout(mainLayout);

	// create labels for outcome
	m_winLabel = new ShadowLabel(GetStringView(lang, StringTableKey::REPLAY_OUTCOME_WIN), 0, 1, 4);
	m_lossLabel = new ShadowLabel(GetStringView(lang, StringTableKey::REPLAY_OUTCOME_LOSS), 0, 1, 4);
	m_drawLabel = new ShadowLabel(GetStringView(lang, StringTableKey::REPLAY_OUTCOME_DRAW), 0, 1, 4);
	m_unknownLabel = new ShadowLabel(GetStringView(lang, StringTableKey::REPLAY_OUTCOME_UNKNOWN), 0, 1, 4);
	m_winLabel->setObjectName("ReplaySummary_winLabel");
	m_lossLabel->setObjectName("ReplaySummary_lossLabel");
	m_drawLabel->setObjectName("ReplaySummary_drawLabel");
	m_unknownLabel->setObjectName("ReplaySummary_unknownLabel");

	// horizontal layout inside the background
	QHBoxLayout* hLayout = new QHBoxLayout();
	hLayout->setContentsMargins(10, 10, 10, 10);
	hLayout->setSpacing(0);

	// vertical layout inside the background
	QVBoxLayout* vLayout = new QVBoxLayout();
	vLayout->setAlignment(Qt::AlignCenter);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->addStretch();

	if (match.Analyzed)
	{
		QHBoxLayout* hatLayout = new QHBoxLayout();
		hatLayout->setObjectName("ReplaySummary_hat");

		const ReplaySummaryData& s = match.ReplaySummary;
		QLabel* outcomeLabel = nullptr;
		switch (s.Outcome)
		{
			case ReplayParser::MatchOutcome::Win:
				outcomeLabel = m_winLabel;
				break;
			case ReplayParser::MatchOutcome::Loss:
				outcomeLabel = m_lossLabel;
				break;
			case ReplayParser::MatchOutcome::Draw:
				outcomeLabel = m_drawLabel;
				break;
			case ReplayParser::MatchOutcome::Unknown:
				outcomeLabel = m_unknownLabel;
				break;
		}
		hatLayout->addWidget(outcomeLabel);
		hatLayout->addStretch();

		/* ----- SCENARIO INFO ----- */
		QVBoxLayout* scenarioInfo = new QVBoxLayout();
		scenarioInfo->setContentsMargins(0, 0, 0, 0);
		scenarioInfo->setSpacing(0);
		ShadowLabel* mapModeLabel = new ShadowLabel(
			fmt::format("{} \u23AF  {}", match.Map, match.MatchGroup).c_str(), 0, 1, 4, QColor(0, 0, 0, 242));
		ShadowLabel* startTimeLabel = new ShadowLabel(GetStringView(lang, StringTableKey::REPLAY_BATTLE_START_TIME), 0, 1, 4, QColor(0, 0, 0, 242));
		ShadowLabel* startTime = new ShadowLabel(match.Date, 0, 1, 4, QColor(0, 0, 0, 242));
		QHBoxLayout* timeLayout = new QHBoxLayout();
		timeLayout->setContentsMargins(0, 0, 0, 0);
		timeLayout->setSpacing(5);
		timeLayout->addWidget(startTimeLabel, 0, Qt::AlignRight | Qt::AlignTop);
		timeLayout->addWidget(startTime, 0, Qt::AlignRight | Qt::AlignTop);
		scenarioInfo->addWidget(mapModeLabel, 0, Qt::AlignRight | Qt::AlignBottom);
		scenarioInfo->addLayout(timeLayout);
		hatLayout->addLayout(scenarioInfo);

		vLayout->addLayout(hatLayout);

		/* ----- PLAYER INFO ----- */
		QHBoxLayout* playerInfo = new QHBoxLayout();
		playerInfo->setContentsMargins(0 ,0 ,0 ,0);
		// playerInfo->setContentsMargins()
		playerInfo->setSpacing(0);

		// ShadowLabel* killerName = new ShadowLabel("razaqq", 0, 1, 4);
		// QLabel* killerShipClass = new QLabel();
		// killerShipClass->setPixmap(QPixmap(fmt::format(":/{}.png", entry.ShipClass).c_str()));
		// ShadowLabel* killerShipTier = new ShadowLabel(TierToString(5), 0, 1, 4);
		// ShadowLabel* killerShipName = new ShadowLabel("Pensacola", 0, 1, 4);
		// playerInfo->addWidget(killerName);
		// playerInfo->addWidget(killerShipClass);
		// playerInfo->addWidget(killerShipTier);
		// playerInfo->addWidget(killerShipName);

		ShadowLabel* playerName = new ShadowLabel(match.Player, 0, 1, 4);
		playerName->setObjectName("ReplaySummary_playerInfoLabel");
		QLabel* shipClass = new QLabel();
		shipClass->setPixmap(QPixmap(fmt::format(":/{}.svg", match.ShipClass).c_str()).scaledToHeight(9, Qt::SmoothTransformation));

		ShadowLabel* shipTier = new ShadowLabel("", 0, 1 ,4);
		shipTier->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		shipTier->setObjectName("ReplaySummary_playerInfoLabel");
		if (match.ShipTier == 11)
		{
			shipTier->setPixmap(QPixmap(":/StarGold.svg").scaledToHeight(13, Qt::SmoothTransformation));
		}
		else
		{
			shipTier->setText(Client::TierToString(match.ShipTier).data());
		}

		ShadowLabel* shipName = new ShadowLabel(match.Ship, 0, 1, 4);
		shipName->setObjectName("ReplaySummary_playerInfoLabel");
		playerInfo->addWidget(playerName);
		playerInfo->addSpacing(5);
		playerInfo->addWidget(shipClass);
		playerInfo->addSpacing(5);
		playerInfo->addWidget(shipTier, 0, Qt::AlignVCenter | Qt::AlignLeft);
		playerInfo->addSpacing(3);
		playerInfo->addWidget(shipName);
		playerInfo->addStretch();

		vLayout->addLayout(playerInfo);
		vLayout->addSpacing(15);

		/* ----- ACHIEVEMENTS ----- */
		if (!s.Achievements.empty())
		{
			ShadowLabel* achievementsLabel = new ShadowLabel(GetStringView(lang, StringTableKey::REPLAY_ACHIEVEMENTS), 0, 1, 4);
			achievementsLabel->setObjectName("ReplaySummary_battlePerformanceLabel");
			vLayout->addWidget(achievementsLabel);

			QHBoxLayout* achievements = new QHBoxLayout();
			for (const auto& [achievement, count] : s.Achievements)
			{
				const QPixmap pixmap(fmt::format(":/{}.png", GetName(achievement)).c_str());
				QLabel* achievementLabel = new QLabel();
				achievementLabel->setPixmap(pixmap.scaledToHeight(53, Qt::SmoothTransformation));

				if (count > 1)
				{
					QVBoxLayout* achievementLayout = new QVBoxLayout();
					QLabel* achievementCountLabel = new QLabel(fmt::format("x{}", count).c_str());
					achievementCountLabel->setObjectName("ReplaySummary_achievementCountLabel");
					achievementLayout->setContentsMargins(0, 0, 0, 0);
					achievementLayout->addWidget(achievementCountLabel, 0, Qt::AlignBottom | Qt::AlignRight);
					achievementLabel->setLayout(achievementLayout);
				}

				achievements->addWidget(achievementLabel, 0, Qt::AlignLeft);
			}
			achievements->addStretch();
			achievements->setSpacing(20);
			achievements->setContentsMargins(0, 0, 0, 0);

			vLayout->addLayout(achievements);
			vLayout->addSpacing(15);
		}

		/* ----- BATTLE PERFORMANCE ----- */
		ShadowLabel* battlePerformanceLabel = new ShadowLabel(GetStringView(lang, StringTableKey::REPLAY_BATTLE_PERFORMANCE), 0, 1, 4);
		battlePerformanceLabel->setObjectName("ReplaySummary_battlePerformanceLabel");

		QFrame* battlePerformance = new QFrame();
		battlePerformance->setObjectName("ReplaySummary_battlePerformance");
		QGridLayout* bpLayout = new QGridLayout();
		bpLayout->setContentsMargins(0, 0, 0, 0);
		bpLayout->setSpacing(5);
		bpLayout->setAlignment(Qt::AlignLeft);
		ShadowLabel* dmgDone = new ShadowLabel(FormatDamageNumber(s.DamageDealt), 1, 1, 1);
		dmgDone->setObjectName("ReplaySummary_DamageLabel");
		bpLayout->addWidget(new Ribbon(dmgDone, GetStringView(lang, StringTableKey::REPLAY_DAMAGE)), 0, 0, Qt::AlignTop | Qt::AlignLeft);

		// group ribbons by parent
		std::unordered_map<RibbonType, uint32_t> ribbons;
		for (const auto& [ribbon, count] : s.Ribbons)
		{
			RibbonType parent = GetParent(ribbon);
			if (ribbons.contains(parent))
			{
				ribbons.at(parent) += count;
			}
			else
			{
				ribbons[parent] = count;
			}
		}

		// add parent ribbons to layout
		int column = 1, row = 0;
		for (const auto& [ribbon, count] : ribbons)
		{
			if (column >= 6)
			{
				column = 0;
				row++;
			}
			const QPixmap pixmap(fmt::format(":/{}.png", GetName(ribbon)).c_str());
			QLabel* ribbonLabel = new QLabel();
			QVBoxLayout* ribbonLayout = new QVBoxLayout();
			ribbonLabel->setPixmap(pixmap.scaledToHeight(40, Qt::SmoothTransformation));
			QLabel* ribbonCountLabel = new QLabel(fmt::format("x{}", count).c_str());
			ribbonCountLabel->setObjectName("ReplaySummary_ribbonCountLabel");
			ribbonLayout->setContentsMargins(1, 1, 2, 0);
			ribbonLayout->addStretch();
			ribbonLayout->addWidget(ribbonCountLabel, 0, Qt::AlignBottom | Qt::AlignRight);
			ribbonLabel->setLayout(ribbonLayout);

			QFlags ribbonAlign = Qt::AlignTop | Qt::AlignLeft;
			if (row > 0 && column == 0)
				ribbonAlign = Qt::AlignTop | Qt::AlignRight;
			bpLayout->addWidget(new Ribbon(ribbonLabel, GetRibbonName(lang, ribbon)), row, column++, ribbonAlign);
		}
		battlePerformance->setLayout(bpLayout);

		vLayout->addWidget(battlePerformanceLabel);
		vLayout->addWidget(battlePerformance);
		vLayout->addSpacing(5);

		/* ----- DETAILED REPORT ----- */
		// ShadowLabel* detailedReportLabel = new ShadowLabel("Detailed Report", 0, 1, 4);
		// detailedReportLabel->setObjectName("ReplaySummary_battlePerformanceLabel");

		QFrame* detailedReport = new QFrame();
		QHBoxLayout* drLayout = new QHBoxLayout();
		drLayout->setContentsMargins(0, 0, 0, 0);
		drLayout->setSpacing(5);
		drLayout->setAlignment(Qt::AlignLeft);
		ShadowLabel* dmgSpotting = new ShadowLabel(FormatDamageNumber(s.DamageSpotting), 1, 1, 1);
		dmgSpotting->setObjectName("ReplaySummary_DamageLabelSmall");
		drLayout->addWidget(new RibbonSmall(dmgSpotting, GetStringView(lang, StringTableKey::REPLAY_SPOTTING_DAMAGE)), 0, Qt::AlignTop | Qt::AlignLeft);
		ShadowLabel* dmgTaken = new ShadowLabel(FormatDamageNumber(s.DamageTaken), 1, 1, 1);
		dmgTaken->setObjectName("ReplaySummary_DamageLabelSmall");
		drLayout->addWidget(new RibbonSmall(dmgTaken, GetStringView(lang, StringTableKey::REPLAY_RECEIVED_DAMAGE)), 0, Qt::AlignTop | Qt::AlignLeft);
		ShadowLabel* dmgPotential = new ShadowLabel(FormatDamageNumber(s.DamagePotential), 1, 1, 1);
		dmgPotential->setObjectName("ReplaySummary_DamageLabelSmall");
		drLayout->addWidget(new RibbonSmall(dmgPotential, GetStringView(lang, StringTableKey::REPLAY_POTENTIAL_DAMAGE)), 0, Qt::AlignTop | Qt::AlignLeft);
		detailedReport->setLayout(drLayout);

		// vLayout->addWidget(detailedReportLabel);
		vLayout->addWidget(detailedReport);
	}

	vLayout->addStretch();

	IconButton* backButton = new IconButton(":/BackArrow.svg", ":/BackArrowHover.svg", QSize(30, 30), false, m_background);
	backButton->setFixedSize(30, 30);
	backButton->move(backButton->mapFromParent(QPoint(20, 20)));
	connect(backButton, &IconButton::clicked, [this]()
	{
		emit ReplaySummaryBack();
	});

	hLayout->addStretch(1);
	hLayout->addLayout(vLayout, 12);
	hLayout->addStretch(1);
	m_background->setLayout(hLayout);
}

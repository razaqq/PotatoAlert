// Copyright 2022 <github.com/razaqq>

#include "Client/MatchHistory.hpp"

#include "Gui/IconButton.hpp"
#include "Gui/ReplaySummary.hpp"

#include <QApplication>
#include <QFontDatabase>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QPainter>


using PotatoAlert::ReplayParser::AchievementType;
using PotatoAlert::ReplayParser::RibbonType;
using PotatoAlert::Client::MatchHistory;
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

static constexpr std::string_view TierToString(uint8_t tier)
{
	switch (tier)
	{
		case 1: return "I";
		case 2: return "II";
		case 3: return "III";
		case 4: return "IV";
		case 5: return "V";
		case 6: return "VI";
		case 7: return "VII";
		case 8: return "VIII";
		case 9: return "IX";
		case 10: return "X";
		default: return "Err";
	}
}

static constexpr std::string_view GetRibbonName(RibbonType ribbon)
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
			return "Target hits";
		case RibbonType::PlaneShotDown:
			return "Aircraft shot down";
		case RibbonType::Incapacitation:
			return "Incapacitations";
		case RibbonType::Destroyed:
		case RibbonType::BuildingDestroyed:
			return "Destroyed";
		case RibbonType::SetFire:
			return "Set on fire";
		case RibbonType::Flooding:
			return "Caused flooding";
		case RibbonType::Citadel:
			return "Hits to citadel";
		case RibbonType::Defended:
			return "Defended";
		case RibbonType::Captured:
			return "Captured";
		case RibbonType::AssistedInCapture:
			return "Assisted in capture";
		case RibbonType::Suppressed:
			return "Suppressed";
		case RibbonType::Spotted:
			return "Spotted";
		case RibbonType::ShotDownByAircraft:
			return "Shot down by fighter";
		case RibbonType::BuffSeized:
			return "Buff picked up";
		default:
			return "Unknown";
	}
}

}  // namespace

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

ReplaySummaryGui::ReplaySummary(QWidget* parent) : QWidget(parent)
{
	QFontDatabase::addApplicationFont(":/WarHeliosCondC.ttf");
	QFontDatabase::addApplicationFont(":/WarHeliosCondCBold.ttf");
}

void ReplaySummaryGui::SetReplaySummary(const MatchHistory::Entry& entry)
{
	ClearLayout(layout());

	m_background = new Background(
		std::format(":/{}.jpg", entry.Map).c_str(), 
		std::format(":/{}.png", entry.ShipNation).c_str()
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
	m_winLabel = new ShadowLabel("Victory!", 0, 1, 4);
	m_lossLabel = new ShadowLabel("Defeat", 0, 1, 4);
	m_drawLabel = new ShadowLabel("Draw", 0, 1, 4);
	m_unknownLabel = new ShadowLabel("Unknown", 0, 1, 4);
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

	if (entry.Analyzed)
	{
		QHBoxLayout* hatLayout = new QHBoxLayout();
		hatLayout->setObjectName("ReplaySummary_hat");

		const ReplaySummaryData& s = entry.ReplaySummary;
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
				std::format("{} \u23AF  {}", entry.Map, entry.MatchGroup).c_str(), 0, 2, 1, QColor(0, 0, 0, 242));
		ShadowLabel* startTimeLabel = new ShadowLabel("Battle Start Time:", 0, 2, 1, QColor(0, 0, 0, 242));
		ShadowLabel* startTime = new ShadowLabel(entry.Date.c_str(), 0, 2, 1, QColor(0, 0, 0, 242));
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
		// killerShipClass->setPixmap(QPixmap(std::format(":/{}.png", entry.ShipClass).c_str()));
		// ShadowLabel* killerShipTier = new ShadowLabel(TierToString(5), 0, 1, 4);
		// ShadowLabel* killerShipName = new ShadowLabel("Pensacola", 0, 1, 4);
		// playerInfo->addWidget(killerName);
		// playerInfo->addWidget(killerShipClass);
		// playerInfo->addWidget(killerShipTier);
		// playerInfo->addWidget(killerShipName);

		ShadowLabel* playerName = new ShadowLabel(entry.Player.c_str(), 0, 1, 4);
		playerName->setObjectName("ReplaySummary_playerInfoLabel");
		QLabel* shipClass = new QLabel();
		shipClass->setPixmap(QPixmap(std::format(":/{}.svg", entry.ShipClass).c_str()).scaledToHeight(9, Qt::SmoothTransformation));

		ShadowLabel* shipTier = new ShadowLabel("", 0, 1 ,4);
		shipTier->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		shipTier->setObjectName("ReplaySummary_playerInfoLabel");
		if (entry.ShipTier == 11)
		{
			shipTier->setPixmap(QPixmap(":/StarGold.svg").scaledToHeight(13, Qt::SmoothTransformation));
		}
		else
		{
			shipTier->setText(TierToString(entry.ShipTier).data());
		}

		ShadowLabel* shipName = new ShadowLabel(entry.Ship.c_str(), 0, 1, 4);
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
			ShadowLabel* achievementsLabel = new ShadowLabel("Achievements", 0, 1, 4);
			achievementsLabel->setObjectName("ReplaySummary_battlePerformanceLabel");
			vLayout->addWidget(achievementsLabel);

			QHBoxLayout* achievements = new QHBoxLayout();
			for (const auto& [achievement, count] : s.Achievements)
			{
				const QPixmap pixmap(std::format(":/{}.png", GetName(achievement)).c_str());
				QLabel* achievementLabel = new QLabel();
				achievementLabel->setPixmap(pixmap.scaledToHeight(53, Qt::SmoothTransformation));

				if (count > 1)
				{
					QVBoxLayout* achievementLayout = new QVBoxLayout();
					QLabel* achievementCountLabel = new QLabel(std::format("x{}", count).c_str());
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
		ShadowLabel* battlePerformanceLabel = new ShadowLabel("Battle Performance", 0, 1, 4);
		battlePerformanceLabel->setObjectName("ReplaySummary_battlePerformanceLabel");

		QFrame* battlePerformance = new QFrame();
		battlePerformance->setObjectName("ReplaySummary_battlePerformance");
		QGridLayout* bpLayout = new QGridLayout();
		bpLayout->setContentsMargins(0, 0, 0, 0);
		bpLayout->setSpacing(5);
		bpLayout->setAlignment(Qt::AlignLeft);
		ShadowLabel* dmgDone = new ShadowLabel(FormatDamageNumber(s.DamageDealt).c_str(), 1, 1, 1);
		dmgDone->setObjectName("ReplaySummary_DamageLabel");
		bpLayout->addWidget(new Ribbon(dmgDone, "Damage"), 0, 0, Qt::AlignTop | Qt::AlignLeft);

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
			const QPixmap pixmap(std::format(":/{}.png", GetName(ribbon)).c_str());
			QLabel* ribbonLabel = new QLabel();
			QVBoxLayout* ribbonLayout = new QVBoxLayout();
			ribbonLabel->setPixmap(pixmap.scaledToHeight(40, Qt::SmoothTransformation));
			QLabel* ribbonCountLabel = new QLabel(std::format("x{}", count).c_str());
			ribbonCountLabel->setObjectName("ReplaySummary_ribbonCountLabel");
			ribbonLayout->setContentsMargins(1, 1, 2, 0);
			ribbonLayout->addStretch();
			ribbonLayout->addWidget(ribbonCountLabel, 0, Qt::AlignBottom | Qt::AlignRight);
			ribbonLabel->setLayout(ribbonLayout);

			QFlags ribbonAlign = Qt::AlignTop | Qt::AlignLeft;
			if (row > 0 && column == 0)
				ribbonAlign = Qt::AlignTop | Qt::AlignRight;
			bpLayout->addWidget(new Ribbon(ribbonLabel, GetRibbonName(ribbon)), row, column++, ribbonAlign);
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
		ShadowLabel* dmgSpotting = new ShadowLabel(FormatDamageNumber(s.DamageSpotting).c_str(), 1, 1, 1);
		dmgSpotting->setObjectName("ReplaySummary_DamageLabelSmall");
		drLayout->addWidget(new RibbonSmall(dmgSpotting, "Spotting DMG"), 0, Qt::AlignTop | Qt::AlignLeft);
		ShadowLabel* dmgTaken = new ShadowLabel(FormatDamageNumber(s.DamageTaken).c_str(), 1, 1, 1);
		dmgTaken->setObjectName("ReplaySummary_DamageLabelSmall");
		drLayout->addWidget(new RibbonSmall(dmgTaken, "DMG Received"), 0, Qt::AlignTop | Qt::AlignLeft);
		ShadowLabel* dmgPotential = new ShadowLabel(FormatDamageNumber(s.DamagePotential).c_str(), 1, 1, 1);
		dmgPotential->setObjectName("ReplaySummary_DamageLabelSmall");
		drLayout->addWidget(new RibbonSmall(dmgPotential, "Potential DMG"), 0, Qt::AlignTop | Qt::AlignLeft);
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
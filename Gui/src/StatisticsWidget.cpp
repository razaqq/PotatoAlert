// Copyright 2023 <github.com/razaqq>

#include "Client/StatsParser.hpp"

#include "Core/Format.hpp"

#include "Gui/StatisticsWidget.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QChart>
#include <QColor>
#include <QChartView>
#include <QFont>
#include <QFormLayout>
#include <QGridLayout>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsTextItem>
#include <QGraphicsSceneEvent>
#include <QtCharts/private/qlegendmarker_p.h>
#include <QtCharts/private/qpielegendmarker_p.h>
#include <QHBoxLayout>
#include <QPieSeries>
#include <QPieLegendMarker>
#include <QLegendMarker>
#include <QTabWidget>
#include <QListWidget>

#include <array>
#include <cmath>
#include <cstdint>
#include <ranges>


using PotatoAlert::Gui::StatWidget;
using PotatoAlert::Gui::StatisticsWidget;
using PotatoAlert::ReplayParser::RibbonType;
using PotatoAlert::ReplayParser::MatchOutcome;

namespace {

static constexpr std::array g_chartColors = {
	"#CB4B4B",
	"#2B591F",
	"#134F90",
	"#BD9B33",
	"#9440ED",
	"#EDC240",
	"#AFD8F8",
	"#CB4B4B",
	"#4DA74D",
	"#9440ED",
};

static constexpr auto PickChartColor(size_t i)
{
	return g_chartColors[i % g_chartColors.size()];
}

template<typename ListValueTo, typename ListValueFrom>
	requires std::derived_from<ListValueTo, ListValueFrom>
static QList<ListValueTo*> ListCast(const QList<ListValueFrom*>& list)
{
	//static_assert(std::is_pointer_v<ListValueFrom> && std::is_pointer_v<ListValueTo>);
	//static_assert(std::derived_from<ListValueTo, ListValueFrom>);
	QList<ListValueTo*> out;
	out.reserve(list.size());
	for (ListValueFrom* from : list)
	{
		out.append(qobject_cast<ListValueTo*>(from));
	}
	return out;
}

static constexpr qreal Distance(const QRectF& rect, const QPointF& point)
{
	const qreal dx = std::max(std::max(rect.left() - point.x(), 0.0), point.x() - rect.right());
	const qreal dy = std::max(std::max(rect.top() - point.y(), 0.0), point.y() - rect.bottom());
	return std::sqrt(dx*dx + dy*dy);
}

static constexpr QVector2D VectorFromDir(float a)
{
	return { std::sin(a), std::cos(a) };
}

// Returns the point nearest to p on rect
static constexpr QPointF NearestPoint(const QRectF& rect, const QPointF& p)
{
	return { 0.0f, 0.0f };
}

// Places the rectangle so that the closest point is dist away from point, returns rect center
static constexpr QPointF PlaceAtDistance(const QPointF& point, const QRectF& rect, float distance, float angle)
{
	// y = mx + b -> b = y - mx
	// x and y are from the point and m is vec.y / vec.x
	// the rectangle has to be placed on this line, but we don't know the distance from point yet
	const QVector2D rectCenterVec = VectorFromDir(angle).normalized();

	float halfDiag = 0.5 * std::sqrt(std::pow(rect.width(), 2.0) + std::pow(rect.height(), 2.0));
	float betaRad = std::atan(rect.height() / rect.width());

	// Vector from rectangle center to the nearest point on R
	float deltaX = halfDiag * std::cos(betaRad + angle);
	float deltaY = halfDiag * std::sin(betaRad + angle);

	if (point.x())
	{
		
	}
	else
	{
		
	}

	return { 0.0f, 0.0f };
}

}  // namespace

class StatLabel : public QLabel
{
public:
	explicit StatLabel(std::string_view text) : QLabel(text.data())
	{
		//static QFont statFont("Helvetica Neue");
		QFont statFont("Noto Sans");
		statFont.setPixelSize(14);
		statFont.setStyleStrategy(QFont::PreferAntialias);
		setFont(statFont);
	}
};

class StatValue : public QLabel
{
public:
	StatValue(std::string_view text, std::string_view color) : QLabel(text.data())
	{
		//static QFont statFont("Helvetica Neue");
		QFont statFont("Noto Sans");
		statFont.setPixelSize(14);
		statFont.setWeight(QFont::Bold);
		statFont.setStyleStrategy(QFont::PreferAntialias);
		setFont(statFont);

		QPalette p;
		p.setColor(QPalette::WindowText, QColor(color.data()));
		setPalette(p);
	}
};

class LegendWidget : public QListWidget
{
public:
	explicit LegendWidget(const QLegend* legend, QWidget* parent = nullptr) : QListWidget(parent), m_legend(legend)
	{
		setMouseTracking(true);
		setSelectionMode(NoSelection);
		viewport()->setAutoFillBackground(false);
		setContentsMargins(0, 0, 0, 0);
		setSpacing(-2);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setFrameStyle(NoFrame);
		setFocusPolicy(Qt::NoFocus);

		for (int i = 0; i < m_legend->markers().size(); i++)
		{
			QPieLegendMarker* marker = qobject_cast<QPieLegendMarker*>(m_legend->markers()[i]);
			QListWidgetItem* newItem = new QListWidgetItem(CreateIcon(marker), marker->label());
			newItem->setFlags(newItem->flags() & ~Qt::ItemIsSelectable);
			newItem->setFont(marker->font());
			addItem(newItem);
			connect(marker, &QLegendMarker::labelChanged, [this, i, marker]()
			{
				item(i)->setText(marker->label());
			});
			connect(marker->series()->slices()[i], &QPieSlice::hovered, [this, i](bool status)
			{
				SetLegendHover(i, status);
			});
		}
	}

private:
	static QIcon CreateIcon(const QLegendMarker* marker)
	{
		QPixmap pix(9, 9);
		pix.fill(Qt::transparent);
		QPainter painter(&pix);
		painter.setBrush(marker->brush());
		painter.setPen(marker->pen());
		painter.drawRect(pix.rect().adjusted(0, 0, -1, -1));
		painter.end();
		return pix;
	}

	void SetChartHover(int index, bool state) const
	{
		QPieLegendMarker* marker = qobject_cast<QPieLegendMarker*>(m_legend->markers()[index]);
		emit marker->series()->slices()[index]->hovered(state);
	}

	void SetLegendHover(int index, bool state) const
	{
		QPieLegendMarker* marker = qobject_cast<QPieLegendMarker*>(m_legend->markers()[index]);
		if (state)
			item(index)->setForeground(marker->brush());
		else
			item(index)->setForeground(palette().text());
	}

protected:
	void mouseMoveEvent(QMouseEvent* event) override
	{
		const int index = indexFromItem(itemAt(event->pos())).row();
		if (index != m_currentHover)
		{
			if (m_currentHover != -1)
				SetChartHover(m_currentHover, false);
			if (index != -1)
				SetChartHover(index, true);
			m_currentHover = index;
		}
	}

	void enterEvent(QEnterEvent* event) override
	{
		if (const QListWidgetItem* item = itemAt(event->position().toPoint()))
		{
			m_currentHover = indexFromItem(item).row();
			SetChartHover(m_currentHover, true);
		}
		QListWidget::enterEvent(event);
	}

	void leaveEvent(QEvent* event) override
	{
		if (m_currentHover != -1)
			SetChartHover(m_currentHover, false);
		QListWidget::leaveEvent(event);
	}

signals:
	void Hovered(bool state);

private:
	int m_currentHover = -1;
	const QLegend* m_legend;
};

class CenteredTextItem : public QGraphicsSimpleTextItem
{
public:
	explicit CenteredTextItem(const QString& text) : m_text(text)
	{
		
	}

	void SetTextColor(const QColor& color)
	{
		m_brush = color;
	}

	void SetBrush(const QBrush& brush)
	{
		m_brush = brush;
	}

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
	{
		const QFontMetrics fm(font());
		auto x = boundingRect();
		const QRect rect = fm.boundingRect(m_text);
		painter->setFont(font());
		painter->setBrush(m_brush);
		painter->drawText(rect, Qt::AlignCenter, m_text);
		painter->drawRect(rect);
		QGraphicsSimpleTextItem::paint(painter, option, widget);
	}

private:
	QBrush m_brush;
	QString m_text;
};

template<typename T>
concept is_number = std::is_floating_point_v<T> || std::is_integral_v<T>;

template<is_number T>
class PieChart : public QChart
{
public:
	PieChart(std::string_view title, std::unordered_map<std::string_view, T>& values)
	{
		QPieSeries* series = new QPieSeries();
		series->setPieSize(m_pieSize);
		series->setHoleSize(0.6);

		auto rv = values | std::views::values;
		const T total = std::accumulate(rv.begin(), rv.end(), (T)0);

		size_t i = 0;
		while (!values.empty())
		{
			auto max = std::ranges::max_element(values, [](const std::pair<std::string_view, T> a, const std::pair<std::string_view, T> b)
			{
				return a.second < b.second;
			});
			auto [name, value] = *max;

			QPieSlice* slice = series->append(
			QString("%1 %2%")
					.arg(name.data())
					.arg(static_cast<float>(value) / total * 100.0f, 0, 'f', 1),
				value);
			slice->setBrush(QColor(PickChartColor(i)));
			slice->setLabelColor(palette().text().color());
			slice->setExplodeDistanceFactor(0.15);
			slice->setLabelArmLengthFactor(0.3);
			connect(slice, &QPieSlice::hovered, [slice](bool state)
			{
				//slice->setLabelVisible(state);
				slice->setExploded(state);
			});
			i++;
			values.erase(max);
		}
		series->setLabelsVisible(false);
		//series->setLabelsPosition(QPieSlice::LabelOutside);
		//series->setHorizontalPosition(0.4f);

		addSeries(series);

		setTheme(ChartThemeLight);
		setAnimationOptions(AllAnimations);

		const QFont titleFont("Noto Sans", 12, QFont::Bold);
		setTitle(title.data());
		setTitleFont(titleFont);
		setTitleBrush(palette().text());

		layout()->setContentsMargins(0, 0, 0, 0);
		setBackgroundRoundness(0);
		setMargins({ 0, 0, 0, 0 });
		setBackgroundVisible(false);

		//QFont legendFont("Helvetica Neue", 10, 700);
		QFont legendFont("Noto Sans", 10, QFont::Bold);
		legendFont.setStyleStrategy(QFont::PreferAntialias);
		legend()->setVisible(false);
		legend()->setAlignment(Qt::AlignLeft);
		legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
		legend()->setShowToolTips(true);
		legend()->setFont(legendFont);
		legend()->layout()->setContentsMargins(0, 0, 0, 0);
		legend()->setBackgroundVisible(false);
		legend()->setLabelColor(palette().text().color());
		//legend()->setBrush(QBrush(QColor(128, 128, 128, 128)));
		//legend()->setPen(QPen(QColor(192, 192, 192, 192)));

		legend()->layout()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		//QGraphicsLinearLayout* legendLayout = new QGraphicsLinearLayout();
		//legendLayout->setSpacing(0);

		for (int i = 0; i < legend()->markers().size(); i++)
		{
			QPieLegendMarker* marker = qobject_cast<QPieLegendMarker*>(legend()->markers()[i]);
			QPieSlice* slice = marker->series()->slices()[i];
			// LegendMarkerItem* item = static_cast<QPieLegendMarkerPrivate*>(static_cast<void*>(marker))->item();
			//QGraphicsLayoutItem* item = legend()->layout()->itemAt(i);
			//legendLayout->addItem(item);
			connect(marker, &QLegendMarker::hovered, [slice, i](bool state)
			{
				emit slice->hovered(state);
			});
		}
	}

	void AddLabels(const QSize& chartSize) const
	{
		const QPieSeries* ser = qobject_cast<QPieSeries*>(series()[0]);
		for (const QPieSlice* slice : ser->slices())
		{
			/*
			 * circle r^2 = x^2 + y^2
			 *
			 */

			if (slice->percentage() <= 0.05)
				continue;

			const QString text(fmt::format("{:.1f}%", slice->percentage() * 100.0).c_str());

			const QFontMetrics fm(slice->labelFont());
			QRect rect = fm.boundingRect(text);

			const qreal dist = std::min(m_pieSize * chartSize.height(), m_pieSize * chartSize.width()) / 2.0 + 20.0;
			//CenteredTextItem* textItem = new CenteredTextItem(text);
			// textItem->SetTextColor(slice->brush().color());
			QGraphicsTextItem* textItem = new QGraphicsTextItem(text);
			textItem->setFont(slice->labelFont());
			textItem->setDefaultTextColor(slice->brush().color());
			const qreal middleAngleRad = qDegreesToRadians(std::fmod(slice->startAngle() + 0.5 * slice->angleSpan(), 360.0));
			const QPointF center = QPointF(ser->horizontalPosition() * chartSize.width(), ser->verticalPosition() * chartSize.height());

			const QPointF p = center + QPointF(qSin(middleAngleRad), -qCos(middleAngleRad)) * dist;
			//scene()->addEllipse(p.x(), p.y(), 2, 2);

			auto c = rect.center();
			textItem->setPos(p - QPointF(rect.center().x(), -rect.y()));
			scene()->addItem(textItem);

			scene()->addEllipse(0, 0, 2, 2);
		}
	}

	bool sceneEvent(QEvent* event) override
	{
		if (!m_sceneAdded)
		{
			m_sceneAdded = true;
			// emit SceneChanged();
		}
		return QChart::sceneEvent(event);
	}

private:
	static constexpr qreal m_pieSize = 0.85;
	bool m_sceneAdded = false;
};

class ChartView : public QWidget
{
public:
	explicit ChartView(QChart* chart, const QSize chartSize = QSize(200, 200), QWidget* parent = nullptr) : QWidget(parent), m_chart(chart)
	{
		// setStyleSheet("border: 1px solid red");

		QVBoxLayout* vLayout = new QVBoxLayout();
		vLayout->setSpacing(0);
		vLayout->setContentsMargins(0, 0, 0, 0);

		chart->setContentsMargins(-10, -10, -10, -10);
		chart->layout()->setContentsMargins(-10, -10, -10, -10);

		QHBoxLayout* hLayout = new QHBoxLayout();
		hLayout->setSpacing(0);
		hLayout->setContentsMargins(0, 0, 0, 0);

		QChartView* view = new QChartView(chart);
		view->setRenderHint(QPainter::Antialiasing);
		view->setFixedSize(chartSize);
		view->viewport()->setAutoFillBackground(false);

		LegendWidget* legend = new LegendWidget(chart->legend());
		legend->setFixedSize(180, 200);

		hLayout->addWidget(legend, 0, Qt::AlignRight);
		hLayout->addWidget(view, 0, Qt::AlignLeft);

		QLabel* title = new QLabel(chart->title());
		title->setFont(chart->titleFont());
		chart->setTitle("");

		vLayout->addWidget(title, 0, Qt::AlignHCenter | Qt::AlignBottom);
		vLayout->addLayout(hLayout);

		setLayout(vLayout);
	}

	[[nodiscard]] QChart* GetChart() const { return m_chart; }

private:
	QChart* m_chart;
};

StatWidget::StatWidget(std::span<const Client::Match> matches, QWidget* parent) : QWidget(parent)
{
	uint32_t totalMatches = 0;
	double totalDamageDealt = 0;
	double totalDamageTaken = 0;
	double totalDamageSpotting = 0;
	double totalDamagePotential = 0;
	uint32_t totalFrags = 0;

	std::unordered_map<std::string_view, uint32_t> types;
	std::unordered_map<std::string_view, uint32_t> tiers;
	std::unordered_map<std::string_view, uint32_t> nations;
	std::unordered_map<std::string_view, uint32_t> matchGroups;
	std::unordered_map<MatchOutcome, uint32_t> outcomes;

	for (const Client::Match& match : matches)
	{
		if (!match.Analyzed)
			continue;

		const ReplaySummary& summary = match.ReplaySummary;
		if (summary.Outcome == MatchOutcome::Unknown)
			continue;
		++outcomes[summary.Outcome];

		totalDamageDealt += summary.DamageDealt;
		totalDamageTaken += summary.DamageTaken;
		totalDamageSpotting += summary.DamageSpotting;
		totalDamagePotential += summary.DamagePotential;
		if (summary.Ribbons.contains(RibbonType::Destroyed))
			totalFrags += summary.Ribbons.at(RibbonType::Destroyed);

		totalMatches++;

		++tiers[Client::TierToString(match.ShipTier)];
		++types[match.ShipClass];
		++nations[match.ShipNation];
		++matchGroups[match.MatchGroup];
	}

	QFormLayout* statsLayout = new QFormLayout();
	statsLayout->addRow(new StatLabel("Battles"), new StatValue(std::to_string(totalMatches), g_chartColors[0]));
	statsLayout->addRow(new StatLabel("Personal Rating"), new StatValue("2456", g_chartColors[0]));
	statsLayout->addRow(new StatLabel("Damage"), new StatValue(fmt::format("{}", std::lround(totalDamageDealt / totalMatches)), g_chartColors[0]));
	statsLayout->addRow(new StatLabel("Win rate"), new StatValue(fmt::format("{:.2f}%", (float)outcomes[MatchOutcome::Win] / (float)totalMatches * 100.0f), g_chartColors[0]));
	statsLayout->addRow(new StatLabel("Destroyed warships"), new StatValue(fmt::format("{:.2f}", (float)totalFrags / (float)totalMatches), g_chartColors[0]));

	QGridLayout* layout = new QGridLayout();
	layout->addLayout(statsLayout, 0, 0, Qt::AlignRight);

	constexpr QSize chartSize(200, 200);

	std::unordered_map<std::string_view, uint32_t> outcome
	{
		{ "Win", outcomes[MatchOutcome::Win] },
		{ "Draw", outcomes[MatchOutcome::Draw] },
		{ "Loss", outcomes[MatchOutcome::Loss] }
	};
	auto winChart = new PieChart("Wins", outcome);
	layout->addWidget(new ChartView(winChart, chartSize), 0, 1, Qt::AlignLeft);
	//winChart->AddLabels(chartSize);

	auto typeChart = new PieChart("Warship Types", types);
	layout->addWidget(new ChartView(typeChart, chartSize), 0, 2, Qt::AlignLeft);
	//typeChart->AddLabels(chartSize);

	auto tierChart = new PieChart("Warship Tiers", tiers);
	layout->addWidget(new ChartView(tierChart, chartSize), 1, 0, Qt::AlignRight);
	//tierChart->AddLabels(chartSize);

	auto nationChart = new PieChart("Warship Nations", nations);
	layout->addWidget(new ChartView(nationChart, chartSize), 1, 1, Qt::AlignLeft);
	//nationChart->AddLabels(chartSize);

	auto matchGroupChart = new PieChart("Match Groups", matchGroups);
	layout->addWidget(new ChartView(matchGroupChart, chartSize), 1, 2, Qt::AlignRight);
	//matchGroupChart->AddLabels(chartSize);

	setLayout(layout);
}

StatisticsWidget::StatisticsWidget(const MatchHistory* matchHistory, QWidget* parent) : QWidget(parent), m_matchHistory(matchHistory)
{
	QFont labelFont("Noto Sans", 13, QFont::Bold);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

	const std::span<const Client::Match> matches = m_matchHistory->GetModel()->GetMatches();

	QHBoxLayout* horLayout = new QHBoxLayout();
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);

	QTabWidget* tabWidget = new QTabWidget(this);
	tabWidget->setObjectName("settingsWidget");

	tabWidget->addTab(new StatWidget(matches), "Overall");
	tabWidget->addTab(new StatWidget(matches.subspan(0, 10)), "Last 10");
	tabWidget->addTab(new StatWidget(matches.subspan(0, 20)), "Last 20");
	tabWidget->addTab(new StatWidget(matches.subspan(0, 50)), "Last 50");
	tabWidget->addTab(new StatWidget(matches.subspan(0, 100)), "Last 100");

	horLayout->addStretch();
	horLayout->addWidget(tabWidget);
	horLayout->addStretch();

	setLayout(horLayout);
}

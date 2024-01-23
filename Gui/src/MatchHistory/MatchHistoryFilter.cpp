// Copyright 2024 <github.com/razaqq>

#include "Client/DatabaseManager.hpp"
#include "Client/StringTable.hpp"

#include "Gui/Events.hpp"
#include "Gui/MatchHistory/MatchHistoryFilter.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#include <set>
#include <span>
#include <string>


using PotatoAlert::Gui::Filter;
using PotatoAlert::Gui::FilterList;
using PotatoAlert::Gui::MatchHistoryFilter;

class FilterModel : public QAbstractListModel
{
public:
	explicit FilterModel(Filter& filter) : m_filter(filter)
	{
	}

	int rowCount(const QModelIndex& parent) const override
	{
		return static_cast<int>(m_filter.size());
	}

	QVariant data(const QModelIndex& index, int role) const override
	{
		if (!index.isValid())
			return QVariant();

		if (role == Qt::DisplayRole)
			return std::next(m_filter.begin(), index.row())->first;

		if (role == Qt::CheckStateRole)
			return std::next(m_filter.begin(), index.row())->second ? Qt::Checked : Qt::Unchecked;

		return QVariant();
	}

	bool setData(const QModelIndex& index, const QVariant& value, int role) override
	{
		if (!index.isValid())
			return false;

		if (role == Qt::CheckStateRole)
		{
			const Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
			m_filter.at(std::next(m_filter.begin(), index.row())->first) = state == Qt::Checked ? true : false;
			emit dataChanged(index, index, { role });
			return true;
		}

		return QAbstractListModel::setData(index, value, role);
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return Qt::NoItemFlags;

		return QAbstractListModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
	}

	void AllDataChanged()
	{
		emit dataChanged(QModelIndex(), QModelIndex());
	}

private:
	Filter& m_filter;
};

FilterList::FilterList(Client::StringTable::StringTableKey key, QWidget* parent) : QWidget(parent), m_groupKey(key)
{
	qApp->installEventFilter(this);

	QListView* view = new QListView();
	FilterModel* model = new FilterModel(m_filter);
	view->setModel(model);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	m_groupBox->setFont(QFont(qApp->font().family(), 11, QFont::Bold));
	QVBoxLayout* groupBoxLayout = new QVBoxLayout();
	groupBoxLayout->setContentsMargins(0, 0, 0, 0);
	m_groupBox->setLayout(groupBoxLayout);
	layout->addWidget(m_groupBox);

	view->setUniformItemSizes(true);
	view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view->setSelectionMode(QAbstractItemView::NoSelection);
	view->setSpacing(2);
	view->setFocusPolicy(Qt::NoFocus);
	groupBoxLayout->addWidget(view);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setContentsMargins(0, 0, 0, 0);

	m_toggle->setObjectName("settingsButton");
	connect(m_toggle, &QPushButton::clicked, [this, model](bool checked)
	{
		const bool anyChecked = std::ranges::any_of(m_filter | std::views::values, [](auto& isChecked){ return isChecked; });
		for (const QString& key : m_filter | std::views::keys)
		{
			m_filter.at(key) = !anyChecked;
		}
		model->AllDataChanged();
	});

	buttonLayout->addWidget(m_toggle);
	layout->addLayout(buttonLayout);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0 ,0)
	connect(model, &FilterModel::dataChanged, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles)
#else
	connect(model, &FilterModel::dataChanged, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
#endif
	{
		emit FilterChanged();
	});
}

bool FilterList::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_groupBox->setTitle(GetString(lang, m_groupKey));
		m_toggle->setText(GetString(lang, Client::StringTable::StringTableKey::HISTORY_TOGGLE));
	}

	return QWidget::eventFilter(watched, event);
}

MatchHistoryFilter::MatchHistoryFilter(QWidget* align, QWidget* parent) : FramelessDialog(parent), m_align(align)
{
	setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
	setWindowModality(Qt::NonModal);

	setFixedHeight(400);

	QHBoxLayout* mainLayout = new QHBoxLayout();

	m_shipList->setFixedWidth(150);
	mainLayout->addWidget(m_shipList);
	connect(m_shipList, &FilterList::FilterChanged, this, &MatchHistoryFilter::FilterChanged);

	m_mapList->setFixedWidth(150);
	mainLayout->addWidget(m_mapList);
	connect(m_mapList, &FilterList::FilterChanged, this, &MatchHistoryFilter::FilterChanged);

	m_modeList->setFixedWidth(100);
	mainLayout->addWidget(m_modeList);
	connect(m_modeList, &FilterList::FilterChanged, this, &MatchHistoryFilter::FilterChanged);

	m_statsModeList->setFixedWidth(100);
	mainLayout->addWidget(m_statsModeList);
	connect(m_statsModeList, &FilterList::FilterChanged, this, &MatchHistoryFilter::FilterChanged);

	m_playerList->setFixedWidth(150);
	mainLayout->addWidget(m_playerList);
	connect(m_playerList, &FilterList::FilterChanged, this, &MatchHistoryFilter::FilterChanged);

	m_regionList->setFixedWidth(100);
	mainLayout->addWidget(m_regionList);
	connect(m_regionList, &FilterList::FilterChanged, this, &MatchHistoryFilter::FilterChanged);

	setLayout(mainLayout);
}

void MatchHistoryFilter::AdjustPosition()
{
	const QPoint topLeft = m_align->mapToGlobal(QPoint(0, 0));
	setGeometry(QRect(topLeft - QPoint(0, height()), QSize(width(), height())));
}

void MatchHistoryFilter::BuildFilter(std::span<const Client::Match> matches) const
{
	m_shipList->Clear();
	m_mapList->Clear();
	m_modeList->Clear();
	m_statsModeList->Clear();
	m_playerList->Clear();
	m_regionList->Clear();

	for (const Client::Match& match : matches)
	{
		m_shipList->AddItem(match.Ship);
		m_mapList->AddItem(match.Map);
		m_modeList->AddItem(match.MatchGroup);
		m_statsModeList->AddItem(match.StatsMode);
		m_playerList->AddItem(match.Player);
		m_regionList->AddItem(match.Region);
	}
}

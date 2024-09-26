// Copyright 2022 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StringTable.hpp"

#include "Core/Time.hpp"
#include "Core/Log.hpp"

#include "Gui/Events.hpp"
#include "Gui/MatchHistory/MatchHistoryModel.hpp"

#include <QAbstractTableModel>
#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QFont>
#include <QString>
#include <QVariant>

#include <cstdint>
#include <chrono>
#include <ranges>
#include <sstream>
#include <chrono>
#include <vector>


using PotatoAlert::Core::Time::TimePoint;
using PotatoAlert::Gui::MatchHistoryModel;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::StringTable::GetString;
using PotatoAlert::Client::StringTable::StringTableKey;

MatchHistoryModel::MatchHistoryModel(const Client::ServiceProvider& serviceProvider, QObject* parent)
	: QAbstractTableModel(parent), m_matches({}), m_services(serviceProvider)
{
	qApp->installEventFilter(this);
}

void MatchHistoryModel::SetReplaySummary(uint32_t id, const ReplaySummary& summary)
{
	const auto it = std::ranges::find_if(m_matches, [id](const Client::Match& match)
	{
		return match.Id == id;
	});

	if (it != std::end(m_matches))
	{
		it->ReplaySummary = summary;
		it->Analyzed = true;
	}
}

std::time_t MatchHistoryModel::GetMatchTime(const Client::Match& match)
{
	if (const std::optional<TimePoint> tp = Core::Time::StrToTime(match.Date, "%Y-%m-%d %H:%M:%S"))
	{
		return tp->time_since_epoch().count();
	}
	if (const std::optional<TimePoint> tp = Core::Time::StrToTime(match.Date, "%d.%m.%Y %H:%M:%S"))
	{
		return tp->time_since_epoch().count();
	}
	LOG_ERROR("Unknown Match::Date in MatchHistoryModel");
	return 0;
}

QVariant MatchHistoryModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (static_cast<size_t>(index.row()) >= m_matches.size() || index.column() >= m_columnCount)
		return QVariant();

	const size_t row = static_cast<size_t>(index.row());
	switch (static_cast<Qt::ItemDataRole>(role))
	{
		case Qt::UserRole:
		{
			if (index.column() == 0)
			{
				return static_cast<qulonglong>(GetMatchTime(m_matches[row]));
			}
			return QVariant();
		}
		case Qt::DisplayRole:
		{
			switch (index.column())
			{
				case 0:
					return QString::fromStdString(m_matches[row].Date);
				case 1:
					return QString::fromStdString(m_matches[row].Ship);
				case 2:
					return QString::fromStdString(m_matches[row].Map);
				case 3:
					return QString::fromStdString(m_matches[row].MatchGroup);
				case 4:
					return QString::fromStdString(m_matches[row].StatsMode);
				case 5:
					return QString::fromStdString(m_matches[row].Player);
				case 6:
					return QString::fromStdString(m_matches[row].Region);
				case 7:
					return m_matches[row].Analyzed;
				default:
					return QVariant();
			}
		}
		case Qt::BackgroundRole:
		{
			switch (m_matches[row].ReplaySummary.Outcome)
			{
				case ReplayParser::MatchOutcome::Win:
					return QColor::fromRgb(23, 209, 51, 50);
				case ReplayParser::MatchOutcome::Loss:
					return QColor::fromRgb(254, 14, 0, 50);
				case ReplayParser::MatchOutcome::Draw:
					return QColor::fromRgb(255, 199, 31, 50);
				case ReplayParser::MatchOutcome::Unknown:
					return QColor::fromRgb(0, 0, 0, 0);
			}
		}
		default:
		{
			return QVariant();
		}
	}
}

QVariant MatchHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (static_cast<Qt::ItemDataRole>(role))
	{
		case Qt::DisplayRole:
		{
			if (orientation == Qt::Horizontal)
			{
				const int lang = m_services.Get<Config>().Get<ConfigKey::Language>();
				switch (section)
				{
					case 0:
						return GetString(lang, StringTableKey::HISTORY_DATE);
					case 1:
						return GetString(lang, StringTableKey::COLUMN_SHIP);
					case 2:
						return GetString(lang, StringTableKey::HISTORY_MAP);
					case 3:
						return GetString(lang, StringTableKey::HISTORY_MODE);
					case 4:
						return GetString(lang, StringTableKey::SETTINGS_STATS_MODE);
					case 5:
						return GetString(lang, StringTableKey::COLUMN_PLAYER);
					case 6:
						return GetString(lang, StringTableKey::HISTORY_REGION);
					default:
						return QVariant();
				}
			}
			return "";
		}
		case Qt::FontRole:
		{
			return QFont(qApp->font().family(), m_headerSize);
		}
		default:
		{
			break;
		}
	}

	return QVariant();
}

bool MatchHistoryModel::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == FontScalingChangeEvent::RegisteredType())
	{
		const float scaling = dynamic_cast<FontScalingChangeEvent*>(event)->GetScaling();
		m_headerSize = (int)std::roundf(11.0f * scaling);
	}
	return QAbstractTableModel::eventFilter(watched, event);
}

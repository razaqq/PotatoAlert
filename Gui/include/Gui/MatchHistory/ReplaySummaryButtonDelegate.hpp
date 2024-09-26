// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/StandardPaths.hpp"

#include <QApplication>
#include <QAbstractItemModel>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionViewItem>


namespace PotatoAlert::Gui {

class ReplaySummaryButtonDelegate : public QAbstractItemDelegate
{
	Q_OBJECT

public:
	explicit ReplaySummaryButtonDelegate(QObject* parent = nullptr) : QAbstractItemDelegate(parent)
	{
	}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		// don't paint anything if not analyzed
		if (!index.data(Qt::DisplayRole).toBool())
		{
			return;
		}

		const QRect rect(option.rect.left() + option.rect.width() - m_width, option.rect.top(), m_width, m_height);


		if (!(option.state & QStyle::State_Selected))
		{
			painter->fillRect(rect, index.data(Qt::BackgroundRole).value<QColor>());
		}

		QStyleOptionButton button;

		if (option.state & QStyle::State_MouseOver)
		{
			button.icon = QIcon(":/ReplaySummaryButtonHover.svg");
		}
		else
		{
			button.icon = QIcon(":/ReplaySummaryButton.svg");
		}

		button.iconSize = QSize(m_width, m_height);
		button.rect = rect;
		button.text = "";
		button.state = QStyle::State_Enabled;

		QApplication::style()->drawControl(QStyle::CE_PushButtonLabel, &button, painter);
	}

	bool editorEvent(QEvent* event, [[maybe_unused]] QAbstractItemModel* model, [[maybe_unused]] const QStyleOptionViewItem& option, const QModelIndex& index) override
	{
		// don't do anything if not analyzed
		if (!index.data(Qt::DisplayRole).toBool())
			return true;

		switch (event->type())
		{
			case QEvent::MouseButtonRelease:
			{
				emit ReplaySummarySelected(index);
				break;
			}
			default:
				break;
		}

		return true;
	}

	QSize sizeHint([[maybe_unused]] const QStyleOptionViewItem& option, [[maybe_unused]] const QModelIndex& index) const override
	{
		return { m_width, m_height };
	}

private:
	static constexpr int m_width = 19;
	static constexpr int m_height = 19;

signals:
	void ReplaySummarySelected(const QModelIndex& index);
};

}  // namespace PotatoAlert::Gui

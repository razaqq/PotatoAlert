// Copyright 2023 <github.com/razaqq>
#pragma once

#include <QApplication>
#include <QHeaderView>
#include <QLayout>
#include <QLayoutItem>
#include <QLabel>
#include <QTableWidget>


namespace PotatoAlert::Gui {

void UpdateWidgetFont(QWidget* widget);

inline void UpdateLayoutFont(QLayout* layout)
{
	if (!layout)
		return;
	for (int i = 0; i < layout->count(); i++)
	{
		QLayoutItem* item = layout->itemAt(i);
		if (QLayout* subLayout = item->layout())
		{
			UpdateLayoutFont(subLayout);
		}
		else if (QWidget* widget = item->widget())
		{
			UpdateWidgetFont(widget);
		}
	}
}

inline void UpdateWidgetFont(QWidget* widget)
{
	if (!widget)
		return;

	const QString family = QApplication::font().family();

	QFont font = widget->font();
	font.setFamily(family);
	widget->setFont(font);

	if (QTableWidget* table = qobject_cast<QTableWidget*>(widget))
	{
		for (int c = 0; c < table->columnCount(); c++)
		{
			QFont f = table->horizontalHeaderItem(c)->font();
			f.setFamily(family);
			table->horizontalHeaderItem(c)->setFont(f);
			for (int r = 0; r < table->rowCount(); r++)
			{
				UpdateWidgetFont(table->cellWidget(r, c));
				if (QTableWidgetItem* item = table->item(r, c))
				{
					QFont itemFont = item->font();
					itemFont.setFamily(family);
					item->setFont(itemFont);
				}
			}
		}
	}
	if (QTabWidget* w = qobject_cast<QTabWidget*>(widget))
	{
		QFont f = w->tabBar()->font();
		f.setFamily(family);
		w->tabBar()->setFont(f);

		for (QObject* o : w->children())
		{
			if (QWidget* child = qobject_cast<QWidget*>(o))
			{
				UpdateWidgetFont(child);
			}
		}
	}
	UpdateLayoutFont(widget->layout());
}


constexpr const char* FontSizeProperty = "baseFontSize";
constexpr int FontSizeRole = Qt::UserRole + 123;

void UpdateWidgetFontScaling(QWidget* widget, float scaling);

inline void UpdateLayoutFontScaling(QLayout* layout, float scaling)
{
	if (!layout)
		return;
	for (int i = 0; i < layout->count(); i++)
	{
		QLayoutItem* item = layout->itemAt(i);
		if (QLayout* subLayout = item->layout())
		{
			UpdateLayoutFontScaling(subLayout, scaling);
		}
		else if (QWidget* widget = item->widget())
		{
			UpdateWidgetFontScaling(widget, scaling);
		}
	}
}

inline void UpdateWidgetFontScaling(QWidget* widget, float scaling)
{
	if (!widget)
		return;

	const QVariant fontSizeProp = widget->property(FontSizeProperty);
	if (fontSizeProp.isValid() && fontSizeProp.canConvert<float>())
	{
		QFont f = widget->font();
		f.setPointSizeF(fontSizeProp.toFloat() * scaling);
		widget->setFont(f);
	}

	if (QTableWidget* table = dynamic_cast<QTableWidget*>(widget))
	{
		for (int c = 0; c < table->columnCount(); c++)
		{
			QTableWidgetItem const* i = table->horizontalHeaderItem(c);
			const QVariant headerBaseSize = i->data(FontSizeRole);
			if (headerBaseSize.canConvert<float>())
			{
				QFont f = i->font();
				f.setPointSizeF(headerBaseSize.toFloat() * scaling);
				table->horizontalHeaderItem(c)->setFont(f);
			}
			for (int r = 0; r < table->rowCount(); r++)
			{
				UpdateWidgetFontScaling(table->cellWidget(r, c), scaling);
				if (QTableWidgetItem* item = table->item(r, c))
				{
					const QVariant fontSizeVariant = item->data(FontSizeRole);
					if (fontSizeVariant.canConvert<float>())
					{
						QFont itemFont = item->font();
						itemFont.setPointSizeF(fontSizeVariant.toFloat() * scaling);
						item->setFont(itemFont);
					}
				}
			}
		}
	}

	for (QObject* o : widget->children())
	{
		if (QWidget* w = dynamic_cast<QWidget*>(o))
			UpdateWidgetFontScaling(w, scaling);
		else if (QLayout* l = dynamic_cast<QLayout*>(o))
			UpdateLayoutFontScaling(l, scaling);
	}
}

}  // namespace PotatoAlert::Gui

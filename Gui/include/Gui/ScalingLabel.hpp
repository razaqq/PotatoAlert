// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Gui/Fonts.hpp"

#include <QApplication>
#include <QLabel>


namespace PotatoAlert::Gui {

class ScalingLabel : public QLabel
{
public:
	explicit ScalingLabel(const QFont& font = QApplication::font(), QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QLabel(parent, f)
	{
		setFont(font);
		setProperty(FontSizeProperty, font.pointSizeF());
	}

	explicit ScalingLabel(const QString& text, const QFont& font = QApplication::font(), QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QLabel(text, parent, f)
	{
		setFont(font);
		setProperty(FontSizeProperty, font.pointSizeF());
	}

	ScalingLabel* SetPointSizeF(float size)
	{
		QFont f = font();
		f.setPointSizeF(size);
		setFont(f);
		setProperty(FontSizeProperty, size);
		return this;
	}
};

}  // namespace PotatoAlert::Gui

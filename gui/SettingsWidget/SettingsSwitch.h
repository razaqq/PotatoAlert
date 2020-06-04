// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QAbstractButton>
#include <QWidget>
#include <QSize>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QBrush>
#include <QString>
#include <QColor>
#include <map>
#include <functional>


typedef std::map<bool, QBrush> brushType;
typedef std::map<bool, QColor> colorType;
typedef std::map<bool, QString> textType;
typedef std::map<bool, std::function<int()>> offsetType;

namespace PotatoAlert {

class SettingsSwitch : public QAbstractButton
{
	Q_OBJECT
	Q_PROPERTY(int offset READ getOffset WRITE setOffset)
public:
	explicit SettingsSwitch(QWidget* parent = nullptr);
public:
	void init();
	void paintEvent(QPaintEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void resizeEvent(QResizeEvent* event);
	void setChecked(bool checked);
	QSize sizeHint();

	int trackRadius;  // track radius > thumb radius
	int thumbRadius;

	int _offset;
	offsetType endOffset;
	int getOffset() const;
	void setOffset(int value);

	brushType thumbColor;
	brushType trackColor;
	colorType textColor;
	textType thumbText;
};

}  // namespace PotatoAlert

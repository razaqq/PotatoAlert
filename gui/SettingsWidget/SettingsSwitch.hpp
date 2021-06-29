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


typedef std::map<bool, QBrush> BrushType;
typedef std::map<bool, QColor> ColorType;
typedef std::map<bool, QString> TextType;
typedef std::map<bool, std::function<int()>> OffsetType;

namespace PotatoAlert {

class SettingsSwitch : public QAbstractButton
{
	Q_OBJECT
	Q_PROPERTY(int offset READ GetOffset WRITE SetOffset)
public:
	explicit SettingsSwitch(QWidget* parent = nullptr);
public:
	void Init();
	void paintEvent(QPaintEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void setChecked(bool checked);
	[[nodiscard]] QSize sizeHint() const override;

	int m_trackRadius;  // track radius > thumb radius
	int m_thumbRadius;

	int m_offset;
	OffsetType m_endOffset;
	[[nodiscard]] int GetOffset() const;
	void SetOffset(int value);

	BrushType m_thumbColor;
	BrushType m_trackColor;
	ColorType m_textColor;
	TextType m_thumbText;
};

}  // namespace PotatoAlert

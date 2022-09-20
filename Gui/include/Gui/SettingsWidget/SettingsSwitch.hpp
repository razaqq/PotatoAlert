// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QAbstractButton>
#include <QBrush>
#include <QColor>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QSize>
#include <QString>
#include <QWidget>

#include <array>
#include <map>


typedef std::map<bool, QBrush> BrushType;
typedef std::map<bool, QString> TextType;

namespace PotatoAlert::Gui {

class SettingsSwitch : public QAbstractButton
{
	Q_OBJECT
	Q_PROPERTY(int Offset READ GetOffset WRITE SetOffset)
	Q_PROPERTY(QColor TrackColor WRITE SetTrackColor MEMBER m_trackColor)
	Q_PROPERTY(QColor ThumbColor WRITE SetThumbColor MEMBER m_thumbColor)

public:
	explicit SettingsSwitch(QWidget* parent = nullptr);

	void Init();
	void paintEvent(QPaintEvent* event) override;
	[[nodiscard]] QSize sizeHint() const override;

private:
	QPropertyAnimation* m_moveAnim = new QPropertyAnimation(this, "Offset");
	QPropertyAnimation* m_trackColorAnim = new QPropertyAnimation(this, "TrackColor");
	QPropertyAnimation* m_thumbColorAnim = new QPropertyAnimation(this, "ThumbColor");

	int m_trackRadius;
	int m_thumbRadius;

	int m_offset;
	QColor m_trackColor;
	QColor m_thumbColor;

	[[nodiscard]] int GetOffset() const;
	void SetOffset(int value);

	void SetTrackColor(const QColor& color)
	{
		m_trackColor = color;
	}
	void SetThumbColor(const QColor& color)
	{
		m_thumbColor = color;
	}
	
	std::array<QColor, 2> m_thumbColors;
	std::array<QColor, 2> m_trackColors;
};

}  // namespace PotatoAlert::Gui

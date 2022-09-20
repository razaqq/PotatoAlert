// Copyright 2020 <github.com/razaqq>

#include "Gui/SettingsWidget/SettingsSwitch.hpp"

#include <QAbstractButton>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QSize>
#include <QSizePolicy>
#include <QWidget>


using PotatoAlert::Gui::SettingsSwitch;

SettingsSwitch::SettingsSwitch(QWidget* parent) : QAbstractButton(parent), m_trackRadius(10), m_thumbRadius(8)
{
	m_offset = m_trackRadius;
	Init();
}

void SettingsSwitch::Init()
{
	setCheckable(true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setCursor(Qt::PointingHandCursor);

	m_moveAnim->setDuration(200);
	m_trackColorAnim->setDuration(200);
	m_thumbColorAnim->setDuration(200);
	
	m_trackColors = { palette().color(QPalette::Base), palette().color(QPalette::Highlight) };
	m_thumbColors = { palette().color(QPalette::Light), QColor::fromRgb(255, 255, 255) };

	m_trackColor = m_trackColors[isChecked()];
	m_thumbColor = m_thumbColors[isChecked()];

	connect(this, &SettingsSwitch::toggled, [this](bool checked)
	{
		m_moveAnim->setStartValue(m_offset);
		m_moveAnim->setEndValue(checked ? width() - m_trackRadius : m_trackRadius);
		m_moveAnim->start();

		m_trackColorAnim->setStartValue(m_trackColor);
		m_trackColorAnim->setEndValue(m_trackColors[checked]);
		m_trackColorAnim->start();

		m_thumbColorAnim->setStartValue(m_thumbColor);
		m_thumbColorAnim->setEndValue(m_thumbColors[checked]);
		m_thumbColorAnim->start();
	});

	setFixedSize(sizeHint());
}

void SettingsSwitch::paintEvent([[maybe_unused]] QPaintEvent* event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true); 
	p.setPen(Qt::NoPen);

	p.setBrush(isEnabled() ? QBrush(m_trackColor) : palette().shadow());
	p.setOpacity(isEnabled() ? 1.0 : 0.8);
	p.drawRoundedRect(
		0, 0,
		width(), height(),
		m_trackRadius, m_trackRadius);

	p.setBrush(isEnabled() ? QBrush(m_thumbColor) : palette().mid());
	p.setOpacity(1.0);
	p.drawEllipse(
		GetOffset() - m_thumbRadius,
		m_trackRadius - m_thumbRadius,
		2 * m_thumbRadius,
		2 * m_thumbRadius
	);
	p.end();
}

QSize SettingsSwitch::sizeHint() const
{
	return { 4 * m_trackRadius, 2 * m_trackRadius };
}

void SettingsSwitch::SetOffset(int value)
{
	m_offset = value;
	update();
}

int SettingsSwitch::GetOffset() const
{
	return m_offset;
}

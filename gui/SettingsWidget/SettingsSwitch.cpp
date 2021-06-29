// Copyright 2020 <github.com/razaqq>

#include <QSize>
#include <QFont>
#include <QWidget>
#include <QAbstractButton>
#include <QSizePolicy>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <map>
#include <functional>
#include "SettingsSwitch.hpp"


using PotatoAlert::SettingsSwitch;

SettingsSwitch::SettingsSwitch(QWidget* parent) : QAbstractButton(parent)
{
	this->m_trackRadius = 10;
	this->m_thumbRadius = 8;
	this->m_offset = this->m_trackRadius;
	this->Init();
}

void SettingsSwitch::Init()
{
	this->setCheckable(true);
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setCursor(Qt::PointingHandCursor);

	std::function<int()> f1 = [this]() { return this->width() - this->m_trackRadius; };
	std::function<int()> f2 = [this]() { return this->m_trackRadius; };
	this->m_endOffset = {
		{ true, f1 },
		{ false, f2 }
	};
	
	this->m_thumbColor = {
		{ true, this->palette().highlightedText() },
		{ false, this->palette().light() }
	};
	this->m_trackColor = {
		{ true, this->palette().highlight() },
		{ false, this->palette().dark() }
	};
	this->m_textColor = {
		{ true, this->palette().highlight().color() },
		{ false, this->palette().dark().color() }
	};
	this->m_thumbText = {
		{ true, QString::fromUtf8("\u2714") },
		{ false, QString::fromUtf8("\u2715") }
	};

	this->setFixedSize(this->sizeHint());
}

void SettingsSwitch::paintEvent(QPaintEvent*)
{
	auto p = new QPainter(this);
	p->setRenderHint(QPainter::Antialiasing, true); 
	p->setPen(Qt::NoPen);

	QBrush trackB;
	QBrush thumbB;
	QColor textC;
	float trackOpacity = 1.0;

	if (this->isEnabled())
	{
		trackB = this->m_trackColor[this->isChecked()];
		thumbB = this->m_thumbColor[this->isChecked()];
		textC = this->m_textColor[this->isChecked()];
	}
	else
	{
		trackB = this->palette().shadow();
		thumbB = this->palette().mid();
		textC = this->palette().shadow().color();
		trackOpacity = 0.8;
	}

	p->setBrush(trackB);
	p->setOpacity(trackOpacity);
	p->drawRoundedRect(
		0, 0,
		this->width(), this->height(),
		this->m_trackRadius, this->m_trackRadius);

	p->setBrush(thumbB);
	p->setOpacity(1.0);
	p->drawEllipse(
			this->GetOffset() - this->m_thumbRadius,
		this->m_trackRadius - this->m_thumbRadius,
		2 * this->m_thumbRadius,
		2 * this->m_thumbRadius);

	p->setPen(textC);
	QFont font = p->font();
	font.setStyleStrategy(QFont::PreferAntialias);
	font.setPixelSize(static_cast<int>(1.5 * this->m_thumbRadius));
	p->setFont(font);
	p->drawText(
		QRectF(
			(double)this->GetOffset() - this->m_thumbRadius,
			(double)this->m_trackRadius - this->m_thumbRadius,
			2 * (double)this->m_thumbRadius,
			2 * (double)this->m_thumbRadius),
		Qt::AlignCenter,
			m_thumbText[this->isChecked()]
	);

	p->end();
}

QSize SettingsSwitch::sizeHint() const
{
	return QSize(4 * this->m_trackRadius, 2 * this->m_trackRadius);
}

void SettingsSwitch::mouseReleaseEvent(QMouseEvent* event)
{
	QAbstractButton::mouseReleaseEvent(event);
	if (event->button() == Qt::LeftButton)
	{
		auto anim = new QPropertyAnimation(this, "offset", this);
		anim->setDuration(120);
		anim->setStartValue(this->GetOffset());
		anim->setEndValue(this->m_endOffset[this->isChecked()]());
		anim->start();
	}
}

void SettingsSwitch::resizeEvent(QResizeEvent* event)
{
	this->SetOffset(this->m_endOffset[this->isChecked()]());
	QAbstractButton::resizeEvent(event);
}

void SettingsSwitch::setChecked(bool checked)
{
	this->SetOffset(this->m_endOffset[checked]());
	QAbstractButton::setChecked(checked);
}

void SettingsSwitch::SetOffset(int value)
{
	this->m_offset = value;
	this->update();
}

int SettingsSwitch::GetOffset() const
{
	return this->m_offset;
}

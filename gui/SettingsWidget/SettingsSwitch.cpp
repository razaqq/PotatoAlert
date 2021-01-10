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
	this->trackRadius = 10;
	this->thumbRadius = 8;
	this->_offset = this->trackRadius;
	this->init();
}

void SettingsSwitch::init()
{
	this->setCheckable(true);
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setCursor(Qt::PointingHandCursor);

	std::function<int()> f1 = [this]() { return this->width() - this->trackRadius; };
	std::function<int()> f2 = [this]() { return this->trackRadius; };
	this->endOffset = {
		{true, f1 },
		{false, f2 }
	};
	
	this->thumbColor = {
		{true, this->palette().highlightedText()},
		{false, this->palette().light()}
	};
	this->trackColor = {
		{true, this->palette().highlight()},
		{false, this->palette().dark()}
	};
	this->textColor = {
		{true, this->palette().highlight().color()},
		{false, this->palette().dark().color()}
	};
	this->thumbText = {
		{true, QString::fromUtf8("\u2714")},
		{false, QString::fromUtf8("\u2715")}
	};

	this->setFixedSize(this->sizeHint());
}

void SettingsSwitch::paintEvent(QPaintEvent* event)
{
	auto p = new QPainter(this);
	p->setRenderHint(QPainter::Antialiasing, true); 
	p->setPen(Qt::NoPen);

	QBrush trackB;
	QBrush thumbB;
	QColor textC;
	float trackOpacity = 1.0;

	if (this->isEnabled()) {
		trackB = this->trackColor[this->isChecked()];
		thumbB = this->thumbColor[this->isChecked()];
		textC = this->textColor[this->isChecked()];
	} else {
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
		this->trackRadius, this->trackRadius
	);

	p->setBrush(thumbB);
	p->setOpacity(1.0);
	p->drawEllipse(
		this->getOffset() - this->thumbRadius,
		this->trackRadius - this->thumbRadius,
		2 * this->thumbRadius,
		2 * this->thumbRadius
	);

	p->setPen(textC);
	QFont font = p->font();
	font.setStyleStrategy(QFont::PreferAntialias);
	font.setPixelSize(static_cast<int>(1.5 * this->thumbRadius));
	p->setFont(font);
	p->drawText(
		QRectF(
			(double)this->getOffset() - this->thumbRadius,
			(double)this->trackRadius - this->thumbRadius,
			2 * (double)this->thumbRadius,
			2 * (double)this->thumbRadius
		),
		Qt::AlignCenter,
		thumbText[this->isChecked()]
	);

	p->end();
}

QSize SettingsSwitch::sizeHint() const
{
	return QSize(4 * this->trackRadius, 2 * this->trackRadius);
}

void SettingsSwitch::mouseReleaseEvent(QMouseEvent* event)
{
	QAbstractButton::mouseReleaseEvent(event);
	if (event->button() == Qt::LeftButton)
	{
		auto anim = new QPropertyAnimation(this, "offset", this);
		anim->setDuration(120);
		anim->setStartValue(this->getOffset());
		anim->setEndValue(this->endOffset[this->isChecked()]());
		anim->start();
	}
}

void SettingsSwitch::resizeEvent(QResizeEvent* event)
{
	this->setOffset(this->endOffset[this->isChecked()]());
	QAbstractButton::resizeEvent(event);
}

void SettingsSwitch::setChecked(bool checked)
{
	this->setOffset(this->endOffset[checked]());
	QAbstractButton::setChecked(checked);
}

void SettingsSwitch::setOffset(int value)
{
	this->_offset = value;
	this->update();
}

int SettingsSwitch::getOffset() const
{
	return this->_offset;
}

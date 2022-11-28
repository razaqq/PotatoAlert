// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"

#include "Gui/AspectRatioWidget.hpp"

#include <QGraphicsDropShadowEffect>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>


namespace PotatoAlert::Gui {

class ShadowLabel : public QLabel
{
public:
	explicit ShadowLabel(std::string_view text, int shadowX = 0, int shadowY = 0, int blurRadius = 0,
						 QColor shadowColor = QColor(0, 0, 0), QWidget* parent = nullptr) : QLabel(text.data(), parent)
	{
		QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
		effect->setBlurRadius(blurRadius);
		effect->setColor(shadowColor);
		effect->setOffset(shadowX, shadowY);
		setGraphicsEffect(effect);
		setObjectName("ReplaySummary_ShadowLabel");
	}
};

class Background : public QWidget
{
public:
	explicit Background(const char* map, const char* nationFlag, QWidget* parent = nullptr)
		: QWidget(parent), m_map(map), m_nationFlag(nationFlag) {}

	void SetImages(const QString& map, const QString& nationFlag)
	{
		m_map = map;
		m_nationFlag = nationFlag;
		update();
		repaint();
	}

protected:
	void paintEvent(QPaintEvent* event) override
	{
		QPainter painter(this);
		QPixmap map = QPixmap::fromImage(QImage(m_map));
		QPixmap flag = QPixmap::fromImage(QImage(m_nationFlag));
		painter.drawPixmap(0, 0, map.scaledToHeight(height(), Qt::SmoothTransformation));
		painter.drawPixmap(0, 0, flag.scaledToHeight(height(), Qt::SmoothTransformation));
		QWidget::paintEvent(event);
	}

private:
	QString m_map;
	QString m_nationFlag;
	static constexpr float m_aspectRatio = 16.0f / 9.0f;
};


class ReplaySummary : public QWidget
{
	Q_OBJECT

public:
	explicit ReplaySummary(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);
	void SetReplaySummary(const Client::Match& match);

private:
	void paintEvent(QPaintEvent* _) override;
	Background* m_background;
	AspectRatioWidget* m_arWidget;
	ShadowLabel* m_winLabel;
	ShadowLabel* m_lossLabel;
	ShadowLabel* m_drawLabel;
	ShadowLabel* m_unknownLabel;
	const Client::ServiceProvider& m_services;

signals:
	void ReplaySummaryBack();
};

}  // namespace PotatoAlert::Gui

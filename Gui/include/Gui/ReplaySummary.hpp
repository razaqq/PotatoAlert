// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"

#include "Gui/AspectRatioWidget.hpp"

#include <QLabel>
#include <QPaintEvent>
#include <QWidget>


namespace PotatoAlert::Gui {

class ShadowLabel;
class Background;

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

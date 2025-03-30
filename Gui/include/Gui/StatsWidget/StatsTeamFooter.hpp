// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Gui/ScalingLabel.hpp"
#include "Gui/StatsParser.hpp"

#include <QEvent>
#include <QFont>
#include <QWidget>


namespace PotatoAlert::Gui {

class StatsTeamFooter : public QWidget
{
public:
	explicit StatsTeamFooter(QWidget* parent = nullptr);
	void Update(const StatsParser::Team& team) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();

private:
	QFont m_labelFont = QFont(QApplication::font().family(), 10, QFont::Bold);
	ScalingLabel* m_wrLabel = new ScalingLabel(m_labelFont);
	ScalingLabel* m_dmgLabel = new ScalingLabel(m_labelFont);
	ScalingLabel* m_regionLabel = new ScalingLabel(m_labelFont);

	ScalingLabel* m_wr = new ScalingLabel("0.0%", m_labelFont);
	ScalingLabel* m_dmg = new ScalingLabel("0", m_labelFont);
	ScalingLabel* m_tag = new ScalingLabel(m_labelFont);
	ScalingLabel* m_name = new ScalingLabel(m_labelFont);
	ScalingLabel* m_region = new ScalingLabel(m_labelFont);
};

}  // namespace PotatoAlert::Gui

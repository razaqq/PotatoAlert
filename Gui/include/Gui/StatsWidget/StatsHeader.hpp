// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"

#include "Gui/ScalingLabel.hpp"

#include <QApplication>
#include <QEvent>
#include <QFont>
#include <QLabel>
#include <QPixmap>
#include <QWidget>

#include <string>


namespace PotatoAlert::Gui {

class StatsHeaderFriendly : public QWidget
{
	Q_OBJECT

public:
	explicit StatsHeaderFriendly(QWidget* parent = nullptr);
	void SetStatus(Client::Status status, std::string_view text) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();

private:
	QLabel* m_statusIcon = new QLabel();
	ScalingLabel* m_statusText = (new ScalingLabel(QFont(QApplication::font().family(), 8)))->SetPointSizeF(7.5f);

	ScalingLabel* m_label = new ScalingLabel(QFont(QApplication::font().family(), 15, QFont::Bold));
	
	QWidget* m_loading;
	QPixmap m_ready;
	QPixmap m_error;
};


class StatsHeaderEnemy : public QWidget
{
	Q_OBJECT

public:
	explicit StatsHeaderEnemy(QWidget* parent = nullptr);
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();

private:
	ScalingLabel* m_label = new ScalingLabel(QFont(QApplication::font().family(), 15, QFont::Bold));
};

}  // namespace PotatoAlert::Gui

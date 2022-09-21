// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"

#include <QEvent>
#include <QLabel>
#include <QPixmap>
#include <QWidget>

#include <string>


using PotatoAlert::Client::Status;

namespace PotatoAlert::Gui {

class StatsHeader : public QWidget
{
	Q_OBJECT
private:
	QLabel* m_statusIcon = new QLabel();
	QLabel* m_statusText = new QLabel();

	QLabel* m_team1Label = new QLabel();
	QLabel* m_team2Label = new QLabel();
	
	QWidget* m_loading;
	QPixmap m_ready;
	QPixmap m_error;

public:
	explicit StatsHeader(QWidget* parent = nullptr);
	void SetStatus(Status status, std::string_view text) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
};

}  // namespace PotatoAlert::Gui

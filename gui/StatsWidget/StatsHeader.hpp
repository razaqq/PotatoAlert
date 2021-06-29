// Copyright 2020 <github.com/razaqq>
#pragma once

#include "PotatoClient.hpp"
#include <QWidget>
#include <QLabel>
#include <QMovie>
#include <QPixmap>
#include <QHBoxLayout>
#include <string>


namespace PotatoAlert {

class StatsHeader : public QWidget
{
	Q_OBJECT
public:
	explicit StatsHeader(QWidget* parent);
	void SetStatus(Status status, const std::string& text);
private:
	void Init();
	void changeEvent(QEvent* event) override;

	QLabel* m_statusIcon = new QLabel();
	QLabel* m_statusText = new QLabel();

	QLabel* m_team1Label = new QLabel();
	QLabel* m_team2Label = new QLabel();

	QMovie* m_loading = new QMovie(":/loading.gif");
	QPixmap m_ready = QPixmap(":/ready.png");
	QPixmap m_error = QPixmap(":/error.png");
};

} // namespace PotatoAlert

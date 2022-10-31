// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"

#include <QEvent>
#include <QLabel>
#include <QPixmap>
#include <QWidget>

#include <string>


namespace PotatoAlert::Gui {

class StatsHeaderFriendly : public QWidget
{
	Q_OBJECT

private:
	QLabel* m_statusIcon = new QLabel();
	QLabel* m_statusText = new QLabel();

	QLabel* m_label = new QLabel();
	
	QWidget* m_loading;
	QPixmap m_ready;
	QPixmap m_error;

public:
	explicit StatsHeaderFriendly(QWidget* parent = nullptr);
	void SetStatus(Client::Status status, std::string_view text) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
};


class StatsHeaderEnemy : public QWidget
{
	Q_OBJECT

private:
	QLabel* m_label = new QLabel();

public:
	explicit StatsHeaderEnemy(QWidget* parent = nullptr);
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
};

}  // namespace PotatoAlert::Gui

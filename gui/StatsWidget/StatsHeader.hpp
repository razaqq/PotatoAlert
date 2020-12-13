// Copyright 2020 <github.com/razaqq>
#pragma once

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
	void setStatus(int statusID, const std::string& statusText);
private:
	void init();
    void changeEvent(QEvent* event) override;

	QLabel* statusIcon = new QLabel;
	QLabel* statusText = new QLabel;

	QLabel* team1Label = new QLabel;
    QLabel* team2Label = new QLabel;

	QMovie* loading = new QMovie(":/loading.gif");
	QPixmap ready = QPixmap(":/ready.png");
	QPixmap error = QPixmap(":/error.png");
};

}  // namespace PotatoAlert

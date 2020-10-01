// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QObject>
#include <QEvent>
#include <QMouseEvent>


namespace PotatoAlert {

class TitleBar : public QWidget
{
	Q_OBJECT
public:
	explicit TitleBar(QWidget* parent);
	int btnStartX();
private:
	QWidget* parentWindow;

	QToolButton* appIcon = new QToolButton(this);
	QLabel* appName = new QLabel(this);
	QToolButton* btnMinimize = new QToolButton(this);
	QToolButton* btnMaximize = new QToolButton(this);
	QToolButton* btnRestore = new QToolButton(this);
	QToolButton* btnClose = new QToolButton(this);

	void init();
	bool eventFilter(QObject* object, QEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;
	void onBtnMinimizeClicked();
	void onBtnMaximizeClicked();
	void onBtnRestoreClicked();
};

}  // namespace PotatoAlert

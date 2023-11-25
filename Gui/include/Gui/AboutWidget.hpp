// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QEvent>
#include <QWidget>


namespace PotatoAlert::Gui {

class AboutWidget : public QWidget
{
public:
	explicit AboutWidget(QWidget* parent = nullptr);
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
};

}  // namespace PotatoAlert::Gui

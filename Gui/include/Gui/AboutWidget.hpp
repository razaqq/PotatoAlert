// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>


namespace PotatoAlert::Gui {

class AboutWidget : public QWidget
{
public:
	explicit AboutWidget(QWidget* parent = nullptr);
private:
	void Init();
};

}  // namespace PotatoAlert::Gui

// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>


namespace PotatoAlert {

class HelpWidget : public QWidget
{
public:
	explicit HelpWidget(QWidget* parent);
private:
	void init();
};

}  // namespace PotatoAlert

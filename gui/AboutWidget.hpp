// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>


namespace PotatoAlert {

class AboutWidget : public QWidget
{
public:
	explicit AboutWidget(QWidget* parent);
private:
	void Init();
};

}  // namespace PotatoAlert

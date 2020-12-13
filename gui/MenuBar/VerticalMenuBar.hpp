// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QButtonGroup>


namespace PotatoAlert {

class VerticalMenuBar : public QWidget
{
public:
	explicit VerticalMenuBar(QWidget* parent);
	QButtonGroup* btnGroup = new QButtonGroup(this);
private:
	void init();
};

}  // namespace PotatoAlert

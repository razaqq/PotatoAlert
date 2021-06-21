// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QDockWidget>
#include <QButtonGroup>


namespace PotatoAlert {

class VerticalMenuBar : public QDockWidget
{
public:
	explicit VerticalMenuBar(QWidget* parent);
	QButtonGroup* btnGroup = new QButtonGroup();
private:
	void Init();
};

}  // namespace PotatoAlert

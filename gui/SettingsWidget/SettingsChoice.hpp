// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QString>
#include <QButtonGroup>


namespace PotatoAlert {

class SettingsChoice : public QWidget
{
public:
	SettingsChoice(QWidget* parent, const std::vector<QString>& buttons);
	QButtonGroup* m_btnGroup;
};

}  // namespace PotatoAlert

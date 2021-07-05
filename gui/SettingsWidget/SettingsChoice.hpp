// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QButtonGroup>
#include <QString>
#include <QWidget>


namespace PotatoAlert {

class SettingsChoice : public QWidget
{
public:
	SettingsChoice(QWidget* parent = nullptr, const std::vector<QString>& buttons = {});
	QButtonGroup* m_btnGroup;
};

}  // namespace PotatoAlert

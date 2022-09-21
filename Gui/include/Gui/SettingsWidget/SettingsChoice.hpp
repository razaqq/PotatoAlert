// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Gui/SettingsWidget/SettingsChoiceButton.hpp"

#include <QButtonGroup>
#include <QFont>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QString>
#include <QWidget>

#include <span>


namespace PotatoAlert::Gui {

class SettingsChoice : public QWidget
{
public:
	SettingsChoice(QWidget* parent = nullptr, std::initializer_list<const QString> buttons = {});
	SettingsChoice(QWidget* parent = nullptr, std::span<const QString> buttons = {});

	QButtonGroup* GetButtonGroup() const { return m_btnGroup; }

private:
	template<typename Iterator>
	void Init(Iterator begin, Iterator end);

	QButtonGroup* m_btnGroup;
	static constexpr int WIDGET_HEIGHT = 20;
};

}  // namespace PotatoAlert::Gui

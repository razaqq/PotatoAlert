// Copyright 2023 <github.com/razaqq>
#pragma once

#include <QComboBox>

#include <span>
#include <string>


namespace PotatoAlert::Gui {

class SettingsComboBox : public QComboBox
{
public:
	explicit SettingsComboBox(std::span<const std::string_view> options)
	{
		setCursor(Qt::PointingHandCursor);
		for (std::string_view opt : options)
			addItem(opt.data());
	}
};

}  // namespace PotatoAlert::Gui

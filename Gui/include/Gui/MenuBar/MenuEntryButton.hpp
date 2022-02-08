// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QIcon>
#include <QPushButton>
#include <QWidget>


namespace PotatoAlert::Gui {

class MenuEntryButton : public QWidget
{
public:
	MenuEntryButton(QWidget* parent = nullptr, const QIcon& icon = QIcon(), bool checkable = true);
	void SetChecked(bool checked) const;
	[[nodiscard]] QPushButton* GetButton() const { return m_button; }

private:
	QPushButton* m_button = new QPushButton();
};

}  // namespace PotatoAlert::Gui

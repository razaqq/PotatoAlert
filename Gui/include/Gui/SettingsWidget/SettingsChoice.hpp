// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Gui/SettingsWidget/SettingsChoiceButton.hpp"

#include <QButtonGroup>
#include <QString>
#include <QWidget>

#include <span>


namespace PotatoAlert::Gui {

class SettingsChoice : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsChoice(QWidget* parent = nullptr, std::initializer_list<const QString> buttons = {});
	explicit SettingsChoice(QWidget* parent = nullptr, std::span<const QString> buttons = {});

	void SetCurrentIndex(int index) const;

private:
	template<typename Iterator>
	void Init(Iterator begin, Iterator end);

	QButtonGroup* m_btnGroup = new QButtonGroup(this);
	static constexpr int WIDGET_HEIGHT = 20;
	bool eventFilter(QObject* watched, QEvent* event) override;
	float m_fontScaling = 1.0f;

signals:
	void CurrentIndexChanged(int index);
};

}  // namespace PotatoAlert::Gui

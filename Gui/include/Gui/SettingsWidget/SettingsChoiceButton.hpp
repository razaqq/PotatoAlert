// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QString>


namespace PotatoAlert::Gui {

class SettingsChoiceButton : public QPushButton
{
	Q_OBJECT

public:
	SettingsChoiceButton(const QString& text, QWidget* parent) : QPushButton(text, parent)
	{
		qApp->installEventFilter(this);

		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

		m_colorNormal = palette().color(QPalette::Base);
		m_colorSelected = palette().color(QPalette::Highlight);
		m_anim->setDuration(200);

		connect(this, &SettingsChoiceButton::toggled, [this](bool checked)
		{
			if (checked)
			{
				m_anim->setStartValue(m_colorNormal);
				m_anim->setEndValue(m_colorSelected);
			}
			else
			{
				m_anim->setStartValue(m_colorSelected);
				m_anim->setEndValue(m_colorNormal);
			}
			m_anim->start();
		});
	}

	Q_PROPERTY(QColor Color READ GetColor WRITE SetColor MEMBER m_color)

	void SetColor(const QColor& color)
	{
		setStyleSheet(std::format("background-color: rgb({}, {}, {})", color.red(), color.green(), color.blue()).c_str());
	}

	bool eventFilter(QObject* watched, QEvent* event) override
	{
		if (event->type() == QEvent::FontChange)
		{
			const int width = fontMetrics().boundingRect(text()).width() + 10;
			setFixedWidth(width);
		}

		return QPushButton::eventFilter(watched, event);
	}

	[[nodiscard]] QColor GetColor() const
	{
		return m_color;
	}

private:
	QPropertyAnimation* m_anim = new QPropertyAnimation(this, "Color");
	QColor m_colorNormal;
	QColor m_colorSelected;
	QColor m_color;
};

}  // namespace PotatoAlert::Gui

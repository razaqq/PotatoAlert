// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include <QDebug>
#include <QResizeEvent>

namespace PotatoAlert::Gui {

class AspectRatioWidget : public QWidget
{
public:
	AspectRatioWidget(QWidget* parent = nullptr, QWidget* widget = new QWidget(), float ratio = 1.0f)
		: QWidget(parent), m_widget(widget), m_ratio(ratio)
	{
		m_layout->setSpacing(0);
		m_layout->setContentsMargins(0, 0, 0, 0);
		setLayout(m_layout);
	}

	[[nodiscard]] QWidget* GetWidget() const { return m_widget; }
	[[nodiscard]] float GetRatio() const { return m_ratio; }

	void SetAspectWidget(QWidget* widget, float ratio = 1.0f)
	{
		m_layout->removeWidget(m_widget);
		m_layout->addWidget(widget);
		m_widget = widget;
		m_ratio = ratio;
	}

	void SetRatio(float ratio)
	{
		m_ratio = ratio;
		update();
	}

protected:
	void resizeEvent([[maybe_unused]] QResizeEvent* event) override
	{
		int w = width();
		int h = height();
		const float ratio = static_cast<float>(h) / static_cast<float>(w);

		if (ratio < m_ratio)
		{
			const int targetWidth = static_cast<int>(static_cast<float>(h) / m_ratio);
			m_widget->setFixedWidth(targetWidth);
			m_widget->setFixedHeight(h);
		}
		else
		{
			const int targetHeight = static_cast<int>(static_cast<float>(w) * m_ratio);
			m_widget->setFixedHeight(targetHeight);
			m_widget->setFixedWidth(w);
		}
	}

private:
	QHBoxLayout* m_layout = new QHBoxLayout();
	QWidget* m_widget;
	float m_ratio;
};

}  // namespace PotatoAlert::Gui

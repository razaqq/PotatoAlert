// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>


namespace PotatoAlert::Gui {

class SettingsSlider : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsSlider(int min, int max, std::string_view suffix = "%", Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr) : QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout();
		setLayout(layout);

		m_slider->setOrientation(orientation);
		m_slider->setRange(min, max);
		m_slider->setTickPosition(QSlider::TicksBelow);
		m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_slider->setTickInterval(10);
		m_slider->setSingleStep(1);

		m_spinBox->setRange(min, max);
		m_spinBox->setSuffix(suffix.data());

		layout->addWidget(m_slider, 0, Qt::AlignLeft);
		layout->addWidget(m_spinBox, 0, Qt::AlignRight);

		connect(m_slider, &QSlider::valueChanged, [this](int value)
		{
			m_spinBox->setValue(value);
		});

		connect(m_slider, &QSlider::sliderReleased, [this]()
		{
			emit ValueChanged(m_slider->value());
		});

		connect(m_spinBox, &QSpinBox::valueChanged, [this](int value)
		{
			m_slider->setValue(value);
		});

		connect(m_spinBox, &QSpinBox::editingFinished, [this]()
		{
			emit ValueChanged(m_spinBox->value());
		});
	}

	void SetValue(int value) const
	{
		m_slider->setValue(value);
		m_spinBox->setValue(value);
	}

	void resizeEvent(QResizeEvent* event) override
	{
		m_slider->setFixedWidth(
			event->size().width()
			- layout()->spacing()
			- layout()->contentsMargins().left()
			- layout()->contentsMargins().right()
			- m_spinBox->width()
		);
	}

private:
	QSpinBox* m_spinBox = new QSpinBox();
	QSlider* m_slider = new QSlider();

signals:
	void ValueChanged(int value);
};

}  // namespace PotatoAlert::Gui

// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/ServiceProvider.hpp"

#include "Gui/Events.hpp"
#include "Gui/Fonts.hpp"
#include "Gui/SettingsWidget/SettingsChoice.hpp"

#include <QApplication>
#include <QButtonGroup>
#include <QFont>
#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QWidget>


using PotatoAlert::Gui::SettingsChoice;

SettingsChoice::SettingsChoice(QWidget* parent, std::initializer_list<const QString> buttons) : QWidget(parent)
{
	Init(buttons.begin(), buttons.end());
}

SettingsChoice::SettingsChoice(QWidget* parent, std::span<const QString> buttons) : QWidget(parent)
{
	Init(buttons.begin(), buttons.end());
}

template<typename Iterator>
void SettingsChoice::Init(Iterator begin, Iterator end)
{
	qApp->installEventFilter(this);

	setObjectName("settingsChoice");

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(1);

	setCursor(Qt::PointingHandCursor);
	setFixedHeight(WIDGET_HEIGHT);

	m_btnGroup->setExclusive(true);

	connect(m_btnGroup, &QButtonGroup::idClicked, [this](int index)
	{
		emit CurrentIndexChanged(index);

		// because we apply a stylesheet on button click, the font size resets every time
		// TODO: maybe report this as a bug to Qt?
		QAbstractButton* button = m_btnGroup->button(index);
		QFont font = button->font();
		font.setPointSizeF(button->property(FontSizeProperty).toFloat() * m_fontScaling);
		button->setFont(font);
	});

	QFont btnFont(QApplication::font().family(), 10, QFont::Bold);
	btnFont.setStyleStrategy(QFont::PreferAntialias);

	size_t i = 0;
	for (auto it = begin; it != end; it++)
	{
		SettingsChoiceButton* button = new SettingsChoiceButton(*it, this);

		static constexpr const char* PosProp = "GroupPosition";
		if (it == begin)
			button->setProperty(PosProp, "First");
		else if (it == end-1)
			button->setProperty(PosProp, "Last");
		else if (begin+1 == end)
			button->setProperty(PosProp, "Single");
		else
			button->setProperty(PosProp, "Middle");

		button->setObjectName("settingsChoiceButton");
		button->setMinimumWidth(5);
		button->setFont(btnFont);
		button->setProperty(FontSizeProperty, btnFont.pointSizeF());
		button->setFixedHeight(WIDGET_HEIGHT);
		button->setFlat(true);
		button->setCheckable(true);

		m_btnGroup->addButton(button, static_cast<int>(i));
		layout->addWidget(button);
		i++;
	}

	setLayout(layout);
}

void SettingsChoice::SetCurrentIndex(int index) const
{
	m_btnGroup->button(index)->setChecked(true);
}

bool SettingsChoice::eventFilter(QObject* watched, QEvent* e)
{
	if (e->type() == FontScalingChangeEvent::RegisteredType())
	{
		m_fontScaling = dynamic_cast<FontScalingChangeEvent*>(e)->GetScaling();
	}
	return QWidget::eventFilter(watched, e);
}

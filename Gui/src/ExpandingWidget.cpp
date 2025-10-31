// Copyright 2024 <github.com/razaqq>

#include "Gui/ExpandingWidget.hpp"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>


using PotatoAlert::Gui::ExpandingWidget;

ExpandingWidget::ExpandingWidget(QLayout* titleLayout, QWidget* parent)
	: QWidget(parent), m_titleLayout(titleLayout)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);

	m_content->setObjectName("ExpandingWidget_Content");
	m_content->setMaximumHeight(0);
	m_content->setMinimumHeight(0);
	m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QHBoxLayout* titleWidgetLayout = new QHBoxLayout();
	titleWidgetLayout->setContentsMargins(0, 0, 0, 0);
	titleWidgetLayout->setSpacing(0);
	titleWidgetLayout->addWidget(m_button);
	titleWidgetLayout->addLayout(m_titleLayout);
	m_titleWidget->setLayout(titleWidgetLayout);
	m_titleWidget->setObjectName("ExpandingWidget_Title");

	layout->addWidget(m_titleWidget, 0);
	layout->addWidget(m_content, 0);

	m_button->setCheckable(false);
	m_button->setStyleSheet("QToolButton { border: none; }");
	m_button->setArrowType(m_isExpanded ? Qt::DownArrow : Qt::RightArrow);
	connect(m_button, &QToolButton::clicked, [this]()
	{
		SetExpanded(!m_isExpanded);
	});
}

void ExpandingWidget::SetContentLayout(QLayout* layout) const
{
	delete m_content->layout();
	m_content->setLayout(layout);

	const int contentHeight = m_content->layout()->sizeHint().height();

	m_contentAnim->setDuration(AnimationDuration);
	m_contentAnim->setStartValue(0);
	m_contentAnim->setEndValue(contentHeight);
	m_contentAnim->setDirection(m_isExpanded ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
	m_contentAnim->start();
}

void ExpandingWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		SetExpanded(!m_isExpanded);
	}
}

void ExpandingWidget::SetExpanded(bool expanded)
{
	m_isExpanded = expanded;
	m_button->setArrowType(m_isExpanded ? Qt::DownArrow : Qt::RightArrow);
	m_button->setChecked(m_isExpanded);
	m_contentAnim->setDirection(m_isExpanded ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
	m_contentAnim->start();
}

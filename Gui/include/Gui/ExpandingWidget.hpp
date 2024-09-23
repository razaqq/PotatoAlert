// Copyright 2024 <github.com/razaqq>
#pragma once

#include <QGridLayout>
#include <QLayout>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QWidget>


namespace PotatoAlert::Gui {

class ExpandingWidget : public QWidget
{
public:
	explicit ExpandingWidget(QLayout* titleLayout, QWidget* parent = nullptr);
	void SetContentLayout(QLayout* layout) const;

protected:
	void mousePressEvent(QMouseEvent* event) override;

private:
	void SetExpanded(bool expanded);

private:
	static constexpr int AnimationDuration = 200;
	bool m_isExpanded = false;
	QToolButton* m_button = new QToolButton(this);
	QWidget* m_content = new QWidget();
	QLayout* m_titleLayout = nullptr;
	QWidget* m_titleWidget = new QWidget();
	QPropertyAnimation* m_contentAnim = new QPropertyAnimation(m_content, "maximumHeight");
};

}  // namespace PotatoAlert::Gui

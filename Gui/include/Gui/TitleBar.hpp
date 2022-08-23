// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QObject>
#include <QToolButton>
#include <QWidget>


namespace PotatoAlert::Gui {

class TitleBar : public QWidget
{
	Q_OBJECT
public:
	explicit TitleBar(QWidget* parent = nullptr);
	[[nodiscard]] QToolButton* GetMaximizeButton() const { return m_btnMaximize; }
	[[nodiscard]] QToolButton* GetMinimizeButton() const { return m_btnMinimize; }
	[[nodiscard]] QToolButton* GetCloseButton() const { return m_btnClose; }
	[[nodiscard]] QToolButton* GetRestoreButton() const { return m_btnRestore; }

private:
	QWidget* m_parentWindow;

	QToolButton* m_appIcon = new QToolButton(this);
	QLabel* m_appName = new QLabel(this);
	QToolButton* m_btnMaximize = new QToolButton(this);
	QToolButton* m_btnMinimize = new QToolButton(this);
	QToolButton* m_btnClose = new QToolButton(this);
	QToolButton* m_btnRestore = new QToolButton(this);

	void Init();
	bool eventFilter(QObject* object, QEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void OnBtnMinimizeClicked() const;
	void OnBtnMaximizeClicked() const;
	void OnBtnRestoreClicked() const;
};

}  // namespace PotatoAlert::Gui

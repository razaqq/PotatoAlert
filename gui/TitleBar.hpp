// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QObject>
#include <QEvent>
#include <QMouseEvent>


namespace PotatoAlert {

class TitleBar : public QWidget
{
	Q_OBJECT
public:
	explicit TitleBar(QWidget* parent = nullptr);
	[[nodiscard]] QObjectList GetIgnores() const { return m_ignore; }
private:
	QWidget* m_parentWindow;

	QToolButton* m_appIcon = new QToolButton(this);
	QLabel* m_appName = new QLabel(this);
	QToolButton* m_btnMinimize = new QToolButton(this);
	QToolButton* m_btnMaximize = new QToolButton(this);
	QToolButton* m_btnRestore = new QToolButton(this);
	QToolButton* m_btnClose = new QToolButton(this);

	void Init();
	bool eventFilter(QObject* object, QEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void OnBtnMinimizeClicked() const;
	void OnBtnMaximizeClicked() const;
	void OnBtnRestoreClicked() const;
	QObjectList m_ignore = { m_btnClose, m_btnMaximize, m_btnMinimize, m_btnRestore };
};

}  // namespace PotatoAlert

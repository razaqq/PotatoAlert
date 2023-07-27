// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QEvent>
#include <QIcon>
#include <QToolButton>
#include <QWidget>

#include <string>


namespace PotatoAlert::Gui {

class IconButton : public QToolButton
{
public:
	explicit IconButton(std::string_view iconPath, std::string_view hoverIconPath, const QSize& size, bool checkable = false, QWidget* parent = nullptr)
		: QToolButton(parent), m_iconSize(size)
	{
		m_icon.addFile(iconPath.data(), m_iconSize);
		m_hoverIcon.addFile(hoverIconPath.data(), m_iconSize);

		setIcon(m_icon);
		setIconSize(m_iconSize);
		setObjectName("IconButton");
		setCursor(Qt::PointingHandCursor);
		setCheckable(checkable);
	}

	[[nodiscard]] bool IsIconShown() const { return m_showIcons; }

	void HideIcon()
	{
		m_showIcons = false;
		setIcon(QIcon());
		setIconSize(m_iconSize);
	}

	void ShowIcon()
	{
		m_showIcons = true;
		setIcon(m_icon);
		setIconSize(m_iconSize);
	}

protected:
	void leaveEvent([[maybe_unused]] QEvent* event) override
	{
		if (m_showIcons)
		{
			setIcon(m_icon);
			setIconSize(m_iconSize);
		}
	}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	void enterEvent([[maybe_unused]] QEnterEvent* event) override
#else
	void enterEvent([[maybe_unused]] QEvent* event) override
#endif
	{
		if (m_showIcons && isEnabled())
		{
			setIcon(m_hoverIcon);
			setIconSize(m_iconSize);
		}
	}

private:
	bool m_showIcons = true;
	QIcon m_icon;
	QSize m_iconSize;
	QIcon m_hoverIcon;
};

}  // namespace PotatoAlert::Gui

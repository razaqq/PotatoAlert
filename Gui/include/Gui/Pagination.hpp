// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Gui/IconButton.hpp"

#include <QButtonGroup>
#include <QFrame>

#include <array>


namespace PotatoAlert::Gui {


class Pagination : public QFrame
{
	Q_OBJECT

private:
	static constexpr int m_displayedPageCount = 7;

	int m_currentPage = 0;
	int m_totalPageCount = 1;
	int m_maxPageDisplay = 5;
	std::array<int, m_displayedPageCount> m_pageNumbers{};
	QButtonGroup* m_buttons = new QButtonGroup();

	IconButton* m_nextButton;
	IconButton* m_previousButton;

public:
	explicit Pagination(QWidget* parent = nullptr);

	void SetCurrentPage(int page);
	void SetTotalPageCount(int count);
	void NextPage();
	void PreviousPage();
	void FirstPage();
	void LastPage();
	int CurrentPage() const;
	bool HasPreviousPage() const;
	bool HasNextPage() const;

private:
	static constexpr int HiddenButton = -2;
	static constexpr int FillerButton = -1;

	void RefreshPageNumbers();
	void RefreshButtons() const;

signals:
	void CurrentPageChanged(int page);
};

}  // namespace PotatoAlert::Gui

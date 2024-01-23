// Copyright 2022 <github.com/razaqq>

#include "Gui/IconButton.hpp"
#include "Gui/Pagination.hpp"

#include <QApplication>
#include <QButtonGroup>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>

#include <algorithm>


using PotatoAlert::Gui::Pagination;

Pagination::Pagination(QWidget* parent) : QFrame(parent)
{
	setAttribute(Qt::WA_Hover);

	m_buttons->setExclusive(true);

	const QFont buttonFont(qApp->font().family(), 10, QFont::DemiBold);

	m_previousButton = new IconButton(":ArrowLeft.svg", ":ArrowLeftHover.svg", QSize(30, 30));
	m_nextButton = new IconButton(":ArrowRight.svg", ":ArrowRightHover.svg", QSize(30, 30));
	m_previousButton->setFixedSize(30, 30);
	m_nextButton->setFixedSize(30, 30);
	m_previousButton->setObjectName("paginationButton");
	m_nextButton->setObjectName("paginationButton");

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(1);

	layout->addWidget(m_previousButton);

	for (int i = 0; i < m_displayedPageCount; i++)
	{
		QPushButton* button = new QPushButton();
		button->setObjectName("paginationButton");
		button->setCheckable(true);
		button->setFixedSize(30, 30);
		button->setChecked(i == 0);
		button->setFont(buttonFont);
		button->setCursor(Qt::PointingHandCursor);
		m_buttons->addButton(button, i);
		layout->addWidget(button);
	}

	layout->addWidget(m_nextButton);

	setLayout(layout);

	connect(m_buttons, &QButtonGroup::idClicked, [this](int id)
	{
		SetCurrentPage(m_pageNumbers[id]);
	});

	connect(m_previousButton, &QPushButton::clicked, [this](bool checked)
	{
		PreviousPage();
	});

	connect(m_nextButton, &QPushButton::clicked, [this](bool checked)
	{
		NextPage();
	});

	SetCurrentPage(0);
}

void Pagination::SetCurrentPage(int page)
{
	m_currentPage = page;

	assert(m_currentPage < m_totalPageCount);

	RefreshPageNumbers();
	RefreshButtons();

	emit CurrentPageChanged(page);
}

void Pagination::SetTotalPageCount(int count)
{
	m_totalPageCount = count;

	SetCurrentPage(std::min(m_currentPage, m_totalPageCount - 1));
}

void Pagination::NextPage()
{
	m_buttons->button(std::min(m_buttons->checkedId() + 1, static_cast<int>(m_buttons->buttons().size() - 1)))->setChecked(true);
	SetCurrentPage(m_currentPage + 1);
}

void Pagination::PreviousPage()
{
	m_buttons->button(std::max(m_buttons->checkedId() - 1, 0))->setChecked(true);
	SetCurrentPage(m_currentPage - 1);
}

void Pagination::FirstPage()
{
	SetCurrentPage(0);
}

void Pagination::LastPage()
{
	SetCurrentPage(m_totalPageCount - 1);
}

int Pagination::CurrentPage() const
{
	return m_currentPage;
}

bool Pagination::HasPreviousPage() const
{
	return m_currentPage > 0;
}

bool Pagination::HasNextPage() const
{
	return m_currentPage < m_totalPageCount - 1;
}

void Pagination::RefreshPageNumbers()
{
	if (m_totalPageCount <= m_displayedPageCount)
	{
		// this means we can fit all pages on the buttons
		for (int i = 0; i < m_displayedPageCount; i++)
		{
			if (i < m_totalPageCount)
			{
				m_pageNumbers[i] = i;
			}
			else
			{
				m_pageNumbers[i] = HiddenButton;
			}
		}
		return;
	}

	// not all pages fit the screen
	m_pageNumbers[0] = 0;
	m_pageNumbers[m_displayedPageCount - 1] = m_totalPageCount - 1;

	int fromIndex = 1;
	int toIndex = m_displayedPageCount - 2;

	const bool middle = m_currentPage >= 3 && m_currentPage < m_totalPageCount - 3;

	if (m_currentPage > 3)
	{
		m_pageNumbers[fromIndex++] = FillerButton;
	}

	if (m_currentPage < m_totalPageCount - 4)
	{
		m_pageNumbers[toIndex--] = FillerButton;
	}

	for (int i = fromIndex; i <= toIndex; i++)
	{
		m_pageNumbers[i] = m_currentPage + i - (middle ? static_cast<int>(std::floor(m_displayedPageCount / 2)) : m_buttons->checkedId());
	}
}

void Pagination::RefreshButtons() const
{
	// refresh previous and next
	m_previousButton->setEnabled(HasPreviousPage());
	m_nextButton->setEnabled(HasNextPage());

	for (int i = 0; i < m_buttons->buttons().size(); i++)
	{
		QAbstractButton* button = m_buttons->button(i);

		if (m_currentPage == m_pageNumbers[i])
		{
			button->setChecked(true);
		}

		if (m_pageNumbers[i] == FillerButton)
		{
			button->setVisible(true);
			button->setEnabled(false);
			button->setText("...");
		}
		else if (m_pageNumbers[i] == HiddenButton)
		{
			button->setVisible(false);
		}
		else
		{
			button->setVisible(true);
			button->setEnabled(true);
			button->setText(QString::number(m_pageNumbers[i] + 1));
		}
	}
}

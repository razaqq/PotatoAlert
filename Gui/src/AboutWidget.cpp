// Copyright 2020 <github.com/razaqq>

#include "Gui/AboutWidget.hpp"
#include "Gui/Fonts.hpp"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QWidget>


using PotatoAlert::Gui::AboutWidget;

AboutWidget::AboutWidget(QWidget* parent) : QWidget(parent)
{
	Init();
}

bool AboutWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::ApplicationFontChange)
	{
		UpdateLayoutFont(layout());
	}
	return QWidget::eventFilter(watched, event);
}

void AboutWidget::Init()
{
	qApp->installEventFilter(this);

	QFont labelFont = QApplication::font();
	labelFont.setPointSize(10);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

	QWidget* centralWidget = new QWidget(this);
	centralWidget->setObjectName("aboutWidget");
	QHBoxLayout* centralLayout = new QHBoxLayout();
	centralLayout->setContentsMargins(10, 10, 10, 10);
	centralLayout->setSpacing(0);

	QHBoxLayout* horLayout = new QHBoxLayout();
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);

	QLabel* appIcon = new QLabel();
	appIcon->setPixmap(QPixmap(":/potato.svg").scaledToWidth(100, Qt::SmoothTransformation));
	horLayout->addWidget(appIcon);

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->setContentsMargins(10, 0, 10, 0);
	gridLayout->setColumnMinimumWidth(0, 100);

	gridLayout->addWidget(new QLabel, 0, 0);  // dummy for alignment
	gridLayout->setRowStretch(0, 1);

	QLabel* authorLabel = new QLabel("Author:");
	authorLabel->setFont(labelFont);
	QLabel* authorText = new QLabel(QApplication::organizationName());
	authorText->setFont(labelFont);
	authorLabel->setBuddy(authorText);
	gridLayout->addWidget(authorLabel, 1, 0);
	gridLayout->addWidget(authorText, 1, 1);

	QLabel* versionLabel = new QLabel("Version:");
	versionLabel->setFont(labelFont);
	QLabel* versionText = new QLabel(QApplication::applicationVersion());
	versionText->setFont(labelFont);
	versionLabel->setBuddy(versionText);
	gridLayout->addWidget(versionLabel, 2, 0);
	gridLayout->addWidget(versionText, 2, 1);

	QLabel* poweredLabel = new QLabel("Powered by:");
	poweredLabel->setFont(labelFont);
	QLabel* poweredText = new QLabel("Qt, rapidjson, tinyxml2, spdlog, zlib, sqlite3");
	poweredText->setFont(labelFont);
	poweredLabel->setBuddy(poweredText);
	gridLayout->addWidget(poweredLabel, 3, 0);
	gridLayout->addWidget(poweredText, 3, 1);

	QLabel* qtLabel = new QLabel("Qt Version:");
	qtLabel->setFont(labelFont);
	QLabel* qtText = new QLabel(qVersion());
	qtText->setFont(labelFont);
	qtLabel->setBuddy(qtText);
	gridLayout->addWidget(qtLabel, 4, 0);
	gridLayout->addWidget(qtText, 4, 1);

	QLabel* licenseLabel = new QLabel("License:");
	licenseLabel->setFont(labelFont);
	QLabel* licenseText = new QLabel("MIT");
	licenseText->setFont(labelFont);
	licenseLabel->setBuddy(licenseText);
	gridLayout->addWidget(licenseLabel, 5, 0);
	gridLayout->addWidget(licenseText, 5, 1);

	gridLayout->addWidget(new QLabel, 6, 0);  // dummy for alignment
	gridLayout->setRowStretch(6, 1);

	horLayout->addLayout(gridLayout);

	centralWidget->setLayout(horLayout);
	centralLayout->addStretch();
	centralLayout->addWidget(centralWidget);
	centralLayout->addStretch();

	setLayout(centralLayout);
}

// Copyright 2020 <github.com/razaqq>

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QApplication>
#include <QPixmap>
#include "AboutWidget.hpp"


using PotatoAlert::AboutWidget;

AboutWidget::AboutWidget(QWidget* parent) : QWidget(parent)
{
	this->init();
}

void AboutWidget::init()
{
    QFont labelFont;
    labelFont.setPointSize(10);
    labelFont.setStyleStrategy(QFont::PreferAntialias);

    auto centralWidget = new QWidget(this);
    centralWidget->setObjectName("aboutWidget");
    auto centralLayout = new QHBoxLayout;
    centralLayout->setContentsMargins(10, 10, 10, 10);
    centralLayout->setSpacing(0);

    auto horLayout = new QHBoxLayout;
    horLayout->setContentsMargins(10, 10, 10, 10);
    horLayout->setSpacing(0);

    auto appIcon = new QLabel;
    appIcon->setPixmap(QPixmap(":/potato.png").scaledToWidth(100));
    horLayout->addWidget(appIcon);

    auto gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(10, 0, 10, 0);
    gridLayout->setColumnMinimumWidth(0, 100);

    gridLayout->addWidget(new QLabel, 0, 0);  // dummy for alignment
    gridLayout->setRowStretch(0, 1);

    auto authorLabel = new QLabel("Author:");
    authorLabel->setFont(labelFont);
    auto authorText = new QLabel(QApplication::organizationName());
    authorText->setFont(labelFont);
    authorLabel->setBuddy(authorText);
    gridLayout->addWidget(authorLabel, 1, 0);
    gridLayout->addWidget(authorText, 1, 1);

    auto versionLabel = new QLabel("Version:");
    versionLabel->setFont(labelFont);
    auto versionText = new QLabel(QApplication::applicationVersion());
    versionText->setFont(labelFont);
    versionLabel->setBuddy(versionText);
    gridLayout->addWidget(versionLabel, 2, 0);
    gridLayout->addWidget(versionText, 2, 1);

    auto poweredLabel = new QLabel("Powered by:");
    poweredLabel->setFont(labelFont);
    auto poweredText = new QLabel("Qt5, nlohmann/json, tinyxml2, fmt");
    poweredText->setFont(labelFont);
    poweredLabel->setBuddy(poweredText);
    gridLayout->addWidget(poweredLabel, 3, 0);
    gridLayout->addWidget(poweredText, 3, 1);

    auto qtLabel = new QLabel("Qt Version:");
    qtLabel->setFont(labelFont);
    auto qtText = new QLabel(qVersion());
    qtText->setFont(labelFont);
    qtLabel->setBuddy(qtText);
    gridLayout->addWidget(qtLabel, 4, 0);
    gridLayout->addWidget(qtText, 4, 1);

    auto licenseLabel = new QLabel("License:");
    licenseLabel->setFont(labelFont);
    auto licenseText = new QLabel("MIT");
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

    this->setLayout(centralLayout);
}

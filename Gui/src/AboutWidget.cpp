// Copyright 2020 <github.com/razaqq>

#include "Gui/AboutWidget.hpp"

#include <QApplication>
#include <QFormLayout>
#include <QLabel>
#include <QPixmap>
#include <QWidget>


using PotatoAlert::Gui::AboutWidget;

AboutWidget::AboutWidget(QWidget* parent) : QWidget(parent)
{
	Init();
}

void AboutWidget::Init()
{
	QFont labelFont;
	labelFont.setPointSize(10);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

	QWidget* centralWidget = new QWidget(this);
	centralWidget->setObjectName("aboutWidget");

	QHBoxLayout* centralLayout = new QHBoxLayout();
	centralLayout->setContentsMargins(10, 10, 10, 10);
	centralLayout->setSpacing(0);

	QFormLayout* vLayout = new QFormLayout();
	vLayout->setContentsMargins(10, 0, 10, 0);
	vLayout->setHorizontalSpacing(10);

	QLabel* appIcon = new QLabel();
	appIcon->setPixmap(QPixmap(":/potato.svg").scaledToWidth(100, Qt::SmoothTransformation));

	QFormLayout* aboutLayout = new QFormLayout();
	aboutLayout->setContentsMargins(0, 0, 0, 0);
	aboutLayout->setHorizontalSpacing(10);

	QLabel* authorLabel = new QLabel("Author:");
	authorLabel->setFont(labelFont);
	QLabel* authorText = new QLabel(QApplication::organizationName());
	authorText->setFont(labelFont);
	aboutLayout->addRow(authorLabel, authorText);

	QLabel* versionLabel = new QLabel("Version:");
	versionLabel->setFont(labelFont);
	QLabel* versionText = new QLabel(QApplication::applicationVersion());
	versionText->setFont(labelFont);
	aboutLayout->addRow(versionLabel, versionText);

	QLabel* poweredLabel = new QLabel("Powered by:");
	poweredLabel->setFont(labelFont);
	QLabel* poweredText = new QLabel("Qt, rapidjson, tinyxml2, spdlog, zlib, sqlite3");
	poweredText->setFont(labelFont);
	aboutLayout->addRow(poweredLabel, poweredText);

	QLabel* qtLabel = new QLabel("Qt Version:");
	qtLabel->setFont(labelFont);
	QLabel* qtText = new QLabel(qVersion());
	qtText->setFont(labelFont);
	aboutLayout->addRow(qtLabel, qtText);

	QLabel* licenseLabel = new QLabel("License:");
	licenseLabel->setFont(labelFont);
	QLabel* licenseText = new QLabel("MIT");
	licenseText->setFont(labelFont);
	aboutLayout->addRow(licenseLabel, licenseText);

	QLabel* donationIcon = new QLabel();
	donationIcon->setPixmap(QPixmap(":/Coffee.svg").scaledToWidth(100, Qt::SmoothTransformation));

	QLabel* donationText = new QLabel();
	donationText->setFont(labelFont);
	donationText->setText("If you like PotatoAlert, you can buy me a coffee<br><a href=\"https://www.paypal.me/potatoalert\">https://www.paypal.me/potatoalert</a>");
	donationText->setTextFormat(Qt::RichText);
	donationText->setTextInteractionFlags(Qt::TextBrowserInteraction);
	donationText->setOpenExternalLinks(true);

	vLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
	vLayout->setFormAlignment(Qt::AlignCenter);
	vLayout->addRow(appIcon, aboutLayout);
	vLayout->addRow(donationIcon, donationText);

	centralWidget->setLayout(vLayout);

	centralLayout->addStretch();
	centralLayout->addWidget(centralWidget);
	centralLayout->addStretch();

	setLayout(centralLayout);
}

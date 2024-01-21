// Copyright 2020 <github.com/razaqq>

#include "Gui/AboutWidget.hpp"
#include "Gui/Fonts.hpp"
#include "Gui/ScalingLabel.hpp"

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

	QFormLayout* vLayout = new QFormLayout();
	vLayout->setContentsMargins(10, 0, 10, 0);
	vLayout->setHorizontalSpacing(10);

	QLabel* appIcon = new QLabel();
	appIcon->setPixmap(QPixmap(":/potato.svg").scaledToWidth(100, Qt::SmoothTransformation));

	QFormLayout* aboutLayout = new QFormLayout();
	aboutLayout->setContentsMargins(0, 0, 0, 0);
	aboutLayout->setHorizontalSpacing(10);

	aboutLayout->addRow(new ScalingLabel("Author:", labelFont), new ScalingLabel(QApplication::organizationName(), labelFont));
	aboutLayout->addRow(new ScalingLabel("Version:", labelFont), new ScalingLabel(QApplication::applicationVersion(), labelFont));
	aboutLayout->addRow(new ScalingLabel("Powered by:", labelFont), new ScalingLabel("Qt, rapidjson, tinyxml2, spdlog, zlib, sqlite3", labelFont));
	aboutLayout->addRow(new ScalingLabel("Qt Version:", labelFont), new ScalingLabel(qVersion(), labelFont));
	aboutLayout->addRow(new ScalingLabel("License:", labelFont), new ScalingLabel("MIT", labelFont));

	ScalingLabel* donationIcon = new ScalingLabel();
	donationIcon->setPixmap(QPixmap(":/Coffee.svg").scaledToWidth(100, Qt::SmoothTransformation));

	ScalingLabel* donationText = new ScalingLabel("If you like PotatoAlert, you can buy me a coffee<br><a href=\"https://www.paypal.me/potatoalert\">https://www.paypal.me/potatoalert</a>", labelFont);
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

// Copyright 2021 <github.com/razaqq>

#include "UpdaterGui.hpp"

#include "StringTable.hpp"
#include "Updater.hpp"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>


using PotatoAlert::Gui::FramelessDialog;
using PotatoUpdater::UpdaterGui;

UpdaterGui::UpdaterGui() : FramelessDialog(nullptr)
{
	auto vLayout = new QVBoxLayout();

	auto waitLabel = new QLabel(GetString(PotatoAlert::StringTable::Keys::UPDATE_DOWNLOADING));

	auto progressBar = new QProgressBar();
	progressBar->setValue(0);
	progressBar->setMaximum(100);

	auto progressLayout = new QHBoxLayout();
	auto progressLabel = new QLabel();
	auto speedLabel = new QLabel();
	progressLayout->addStretch();
	progressLayout->addWidget(progressLabel);
	progressLayout->addStretch();
	progressLayout->addWidget(speedLabel);
	progressLayout->addStretch();

	vLayout->addWidget(waitLabel, 0, Qt::AlignHCenter);
	vLayout->addWidget(progressBar);
	vLayout->addLayout(progressLayout);
	this->setLayout(vLayout);

	connect(&Updater::Instance(), &Updater::DownloadProgress,
			[progressBar, progressLabel, speedLabel](int percent, const QString& progress, const QString& speed)
			{
				progressBar->setValue(percent);
				progressLabel->setText(progress);
				speedLabel->setText(speed);
			});

	/*
	connect(updater, &Updater::errorOccurred, [this](const QString& text)
	{
		// clear dialog from progress
		qDeleteAll(this->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));
		delete this->layout();

		auto errorLabel = new QLabel(PotatoAlert::Get(PotatoAlert::Keys::UPDATE_FAILED) + text);
		errorLabel->setWordWrap(true);

		auto errorIcon = new QLabel();
		auto errorPix = QPixmap(":/error.png").scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		errorIcon->setPixmap(errorPix);

		auto hLayout = new QHBoxLayout();
		hLayout->addStretch();
		hLayout->addWidget(errorIcon);
		hLayout->addStretch();
		hLayout->addWidget(errorLabel);
		hLayout->addStretch();

		auto okButton = new QPushButton(PotatoAlert::Get(PotatoAlert::Keys::OK));
		okButton->setObjectName("confirmButton");
		connect(okButton, &QPushButton::clicked, [this]() { this->close(); });

		auto vLayout = new QVBoxLayout();
		vLayout->addLayout(hLayout);
		vLayout->addWidget(okButton);

		this->setLayout(vLayout);
	});
	*/

	Updater::Instance().Run();
	this->exec();
}

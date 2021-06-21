// Copyright 2021 <github.com/razaqq>

#include "UpdaterGui.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include "StringTable.hpp"

using PotatoAlert::FramelessDialog;
using PotatoUpdater::UpdaterGui;

UpdaterGui::UpdaterGui(Updater* updater) : FramelessDialog(nullptr)
{
	auto vLayout = new QVBoxLayout();

	auto waitLabel = new QLabel(PotatoAlert::GetString(PotatoAlert::StringKeys::UPDATE_DOWNLOADING));

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

	connect(updater, &Updater::downloadProgress,
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

		auto errorLabel = new QLabel(PotatoAlert::GetString(PotatoAlert::StringKeys::UPDATE_FAILED) + text);
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

		auto okButton = new QPushButton(PotatoAlert::GetString(PotatoAlert::StringKeys::OK));
		okButton->setObjectName("confirmButton");
		connect(okButton, &QPushButton::clicked, [this]() { this->close(); });

		auto vLayout = new QVBoxLayout();
		vLayout->addLayout(hLayout);
		vLayout->addWidget(okButton);

		this->setLayout(vLayout);
	});
	*/

	updater->Run();
	this->exec();
}

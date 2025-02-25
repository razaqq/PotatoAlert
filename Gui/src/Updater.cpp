// Copyright 2021 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StringTable.hpp"
#include "Client/Updater.hpp"

#include "Gui/FramelessDialog.hpp"
#include "Gui/Updater.hpp"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>


using namespace PotatoAlert::Client::StringTable;
using namespace PotatoAlert::Core;
using PotatoAlert::Gui::FramelessDialog;
using UpdaterGui = PotatoAlert::Gui::Updater;
using UpdaterClient = PotatoAlert::Client::Updater;

UpdaterGui::Updater(const Client::ServiceProvider& serviceProvider) : FramelessDialog(nullptr)
{
	QVBoxLayout* vLayout = new QVBoxLayout();

	int lang = serviceProvider.Get<Client::Config>().Get<Client::ConfigKey::Language>();
	QLabel* waitLabel = new QLabel(GetString(lang, StringTableKey::UPDATE_DOWNLOADING));

	QProgressBar* progressBar = new QProgressBar();
	progressBar->setValue(0);
	progressBar->setMaximum(100);

	QHBoxLayout* progressLayout = new QHBoxLayout();
	QLabel* progressLabel = new QLabel();
	QLabel* speedLabel = new QLabel();
	progressLayout->addStretch();
	progressLayout->addWidget(progressLabel);
	progressLayout->addStretch();
	progressLayout->addWidget(speedLabel);
	progressLayout->addStretch();

	vLayout->addWidget(waitLabel, 0, Qt::AlignHCenter);
	vLayout->addWidget(progressBar);
	vLayout->addLayout(progressLayout);
	setLayout(vLayout);

	UpdaterClient& updater = serviceProvider.Get<UpdaterClient>();

	connect(&updater, &UpdaterClient::DownloadProgress,
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

	updater.Run();
	QDialog::exec();  // TODO
}

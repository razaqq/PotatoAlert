// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"

#include <QWidget>

#include <span>


namespace PotatoAlert::Gui {

class GameInstalls : public QWidget
{
	Q_OBJECT

public:
	explicit GameInstalls(QWidget* parent = nullptr);
	void SetInstalls(std::span<const Client::GameDirectory> infos) const;

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	int m_currentLang = 0;

signals:
	void RemoveGameInstall(const Client::GameDirectory& game) const;
};

}  // namespace PotatoAlert::Gui

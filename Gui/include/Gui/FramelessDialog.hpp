// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QDialog>
#include <QShowEvent>
#include <QWidget>


namespace PotatoAlert::Gui {

class FramelessDialog : public QDialog
{
public:
	explicit FramelessDialog(QWidget* parent = nullptr);

private:
	void showEvent(QShowEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
#else
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
#endif
};

}  // namespace PotatoAlert::Gui

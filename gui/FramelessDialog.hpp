// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QDialog>
#include <QWidget>
#include <QFocusEvent>
#include <QShowEvent>


namespace PotatoAlert {

class FramelessDialog : public QDialog
{
public:
	explicit FramelessDialog(QWidget* parent);
private:
	void showEvent(QShowEvent* event) override;
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
};

}  // namespace PotatoAlert

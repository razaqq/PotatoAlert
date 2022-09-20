// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Gui/FramelessDialog.hpp"


namespace PotatoAlert::Gui {

enum class QuestionAnswer
{
	Yes,
	No
};

class QuestionDialog : public FramelessDialog
{
public:
	QuestionDialog(int language, QWidget* parent = nullptr, const QString& questionText = "");
	QuestionAnswer Run();
};

}  // namespace PotatoAlert::Gui

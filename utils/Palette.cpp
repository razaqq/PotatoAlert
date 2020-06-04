// Copyright 2020 <github.com/razaqq>

#include <QColor>
#include <QPalette>
#include "Palette.h"


QPalette dark()
{
	QPalette p;

    p.setColor(QPalette::WindowText, QColor(180, 180, 180));
    p.setColor(QPalette::Button, QColor(53, 53, 53));
    p.setColor(QPalette::Light, QColor(180, 180, 180));
    p.setColor(QPalette::Midlight, QColor(90, 90, 90));
    p.setColor(QPalette::Dark, QColor(35, 35, 35));
    p.setColor(QPalette::Text, QColor(180, 180, 180));
    p.setColor(QPalette::BrightText, QColor(180, 180, 180));
    p.setColor(QPalette::ButtonText, QColor(180, 180, 180));
    p.setColor(QPalette::Base, QColor(42, 42, 42));
    p.setColor(QPalette::Window, QColor(53, 53, 53));
    p.setColor(QPalette::Shadow, QColor(20, 20, 20));
    p.setColor(QPalette::Highlight, QColor(42, 130, 218));
    p.setColor(QPalette::HighlightedText, QColor(180, 180, 180));
    p.setColor(QPalette::Link, QColor(56, 252, 196));
    p.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    p.setColor(QPalette::ToolTipBase, QColor(53, 53, 53));
    p.setColor(QPalette::ToolTipText, QColor(180, 180, 180));
    p.setColor(QPalette::LinkVisited, QColor(80, 80, 80));

    // disabled
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    p.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

	return p;
}

QPalette light()
{
	QPalette p;

    p.setColor(QPalette::WindowText, QColor(0, 0, 0));
    p.setColor(QPalette::Button, QColor(240, 240, 240));
    p.setColor(QPalette::Light, QColor(180, 180, 180));
    p.setColor(QPalette::Midlight, QColor(200, 200, 200));
    p.setColor(QPalette::Dark, QColor(225, 225, 225));
    p.setColor(QPalette::Text, QColor(0, 0, 0));
    p.setColor(QPalette::BrightText, QColor(0, 0, 0));
    p.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    p.setColor(QPalette::Base, QColor(237, 237, 237));
    p.setColor(QPalette::Window, QColor(240, 240, 240));
    p.setColor(QPalette::Shadow, QColor(20, 20, 20));
    p.setColor(QPalette::Highlight, QColor(76, 163, 224));
    p.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    p.setColor(QPalette::Link, QColor(0, 162, 232));
    p.setColor(QPalette::AlternateBase, QColor(225, 225, 225));
    p.setColor(QPalette::ToolTipBase, QColor(240, 240, 240));
    p.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    p.setColor(QPalette::LinkVisited, QColor(222, 222, 222));

    // disabled
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(115, 115, 115));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(115, 115, 115));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(115, 115, 115));
    p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(190, 190, 190));
    p.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(115, 115, 115));

	return p;
}

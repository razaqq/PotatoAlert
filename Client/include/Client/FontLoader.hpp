// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Log.hpp"
#include "Core/Process.hpp"

#include <QFontDatabase>

#include <array>
#include <string>


namespace PotatoAlert::Client {

constexpr std::array<const std::string_view, 7> Fonts =
{
	"Lato", "NotoSans", "NunitoSans", "Open Sans", "Poppins", "Roboto", "Segoe UI"
};

inline void LoadFonts()
{
#define LOAD_FONT(Name)                                \
	if (QFontDatabase::addApplicationFont(Name) == -1) \
	{                                                  \
		LOG_ERROR("Failed to load font: {}", Name);    \
		Core::ExitCurrentProcessWithError(1);          \
	}                                                  \
	(void)0

	LOAD_FONT(":/Lato-Bold.ttf");
	LOAD_FONT(":/Lato-Light.ttf");
	LOAD_FONT(":/Lato-Regular.ttf");

	LOAD_FONT(":/NotoSans-Light.ttf");
	LOAD_FONT(":/NotoSans-Regular.ttf");
	LOAD_FONT(":/NotoSans-Medium.ttf");
	LOAD_FONT(":/NotoSans-Bold.ttf");

	LOAD_FONT(":/NunitoSans-Light.ttf");
	LOAD_FONT(":/NunitoSans-Regular.ttf");
	LOAD_FONT(":/NunitoSans-Medium.ttf");
	LOAD_FONT(":/NunitoSans-Bold.ttf");

	LOAD_FONT(":/OpenSans-Light.ttf");
	LOAD_FONT(":/OpenSans-Regular.ttf");
	LOAD_FONT(":/OpenSans-Medium.ttf");
	LOAD_FONT(":/OpenSans-Bold.ttf");

	LOAD_FONT(":/Poppins-Light.ttf");
	LOAD_FONT(":/Poppins-Regular.ttf");
	LOAD_FONT(":/Poppins-Medium.ttf");
	LOAD_FONT(":/Poppins-Bold.ttf");

	LOAD_FONT(":/Roboto-Light.ttf");
	LOAD_FONT(":/Roboto-Regular.ttf");
	LOAD_FONT(":/Roboto-Medium.ttf");
	LOAD_FONT(":/Roboto-Bold.ttf");

#undef LOAD_FONT
}

}  // namespace PotatoAlert::Client

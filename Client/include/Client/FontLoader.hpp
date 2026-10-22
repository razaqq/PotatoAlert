// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Log.hpp"
#include "Core/Process.hpp"

#include <QFontDatabase>
#include <QStringList>

#include <array>
#include <string>


namespace PotatoAlert::Client {

constexpr std::array<const std::string_view, 8> Fonts =
{
	"Lato", "LXGW WenKai Lite", "NotoSans", "NunitoSans", "Open Sans", "Poppins", "Roboto", "Segoe UI"
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

	// LXGW WenKai Lite from github.com/lxgw/LxgwWenKai-Lite (see Resources/Fonts/LXGWWenKaiLite-LICENSE.txt)
	LOAD_FONT(":/LXGWWenKaiLite-Regular.ttf");
	LOAD_FONT(":/LXGWWenKaiLite-Medium.ttf");

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

	// Subsets of Noto Sans CJK from github.com/notofonts/noto-cjk, made by Resources/Fonts/subset_noto_cjk.py (see Resources/Fonts/NotoSansCJK-LICENSE.txt)
	LOAD_FONT(":/NotoSansCJKSC-Regular.otf");
	LOAD_FONT(":/NotoSansCJKSC-Bold.otf");
	LOAD_FONT(":/NotoSansCJKTC-Regular.otf");
	LOAD_FONT(":/NotoSansCJKTC-Bold.otf");
	LOAD_FONT(":/NotoSansCJKJP-Regular.otf");
	LOAD_FONT(":/NotoSansCJKJP-Bold.otf");
	LOAD_FONT(":/NotoSansCJKKR-Regular.otf");
	LOAD_FONT(":/NotoSansCJKKR-Bold.otf");

#undef LOAD_FONT

	QFontDatabase::addApplicationFallbackFontFamily(QChar::Script_Hiragana, "Noto Sans CJK JP");
	QFontDatabase::addApplicationFallbackFontFamily(QChar::Script_Katakana, "Noto Sans CJK JP");
	QFontDatabase::addApplicationFallbackFontFamily(QChar::Script_Hangul, "Noto Sans CJK KR");
}

// Han characters are written differently in Simplified Chinese, Traditional Chinese and Japanese, so they follow the UI language
// Each subset only covers its region's common characters, so the other regions fill in the rest
inline void UpdateHanFallbackFonts(std::string_view language)
{
	if (language == "日本語")
		QFontDatabase::setApplicationFallbackFontFamilies(QChar::Script_Han, { "Noto Sans CJK JP", "Noto Sans CJK TC", "Noto Sans CJK SC" });
	else if (language == "简体中文")
		QFontDatabase::setApplicationFallbackFontFamilies(QChar::Script_Han, { "Noto Sans CJK SC", "Noto Sans CJK TC", "Noto Sans CJK JP" });
	else
		QFontDatabase::setApplicationFallbackFontFamilies(QChar::Script_Han, { "Noto Sans CJK TC", "Noto Sans CJK SC", "Noto Sans CJK JP" });
}

}  // namespace PotatoAlert::Client

// Copyright 2020 <github.com/razaqq>
#pragma once

#include "StatsParser.hpp"
#include <string>


using PotatoAlert::StatsParser::Match;

namespace PotatoAlert::CSV {

QString GetDir();
void SaveMatch(const std::string& csv);

}  // namespace PotatoAlert::CSV

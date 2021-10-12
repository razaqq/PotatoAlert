// Copyright 2021 <github.com/razaqq>

#include "ReplayParser.hpp"

#include "catch.hpp"
#include "win32.h"

#include <filesystem>
#include <iostream>
#include <string>


using namespace PotatoAlert::ReplayParser;
namespace fs = std::filesystem;


static std::string GetReplay(std::string_view name)
{
	char path[MAX_PATH];
	if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
	{
		exit(1);
	}

	return (fs::path(path).remove_filename() / "replays" / name).string();
}

TEST_CASE( "ReplayTest" )
{
	std::optional<ReplayFile> res = ReplayFile::FromFile(GetReplay("20201107_155356_PISC110-Venezia_19_OC_prey.wowsreplay"));
	REQUIRE(res.has_value());
	ReplayFile replay = res.value();
	REQUIRE(replay.meta.name == "12x12");
	REQUIRE(replay.meta.dateTime == "07.11.2020 15:53:56");
	REQUIRE(replay.packets.size() == 153376);
}

TEST_CASE( "ReplayTest2" )
{
	std::optional<ReplayFile> res = ReplayFile::FromFile(GetReplay("20201107_155356_PISC110-Venezia_19_OC_prey.wowsreplay"));  // WIN
	REQUIRE(res.has_value());
	ReplayFile replay1 = res.value();
	auto wonRes = replay1.Won();
	REQUIRE((wonRes && wonRes.value()));

	res = ReplayFile::FromFile(GetReplay("20210914_212320_PRSC610-Smolensk_25_sea_hope.wowsreplay"));  // WIN
	REQUIRE(res.has_value());
	ReplayFile replay2 = res.value();
	auto wonRes2 = replay2.Won();
	REQUIRE((wonRes2 && wonRes2.value()));

	res = ReplayFile::FromFile(GetReplay("20210913_011502_PASD510-Somers_53_Shoreside.wowsreplay"));  // LOSS
	REQUIRE(res.has_value());
	ReplayFile replay3 = res.value();
	auto wonRes3 = replay3.Won();
	REQUIRE((wonRes3 && !wonRes3.value()));

	res = ReplayFile::FromFile(GetReplay("20210912_002554_PRSB110-Sovetskaya-Rossiya_53_Shoreside.wowsreplay"));  // WIN
	REQUIRE(res.has_value());
	ReplayFile replay4 = res.value();
	auto wonRes4 = replay4.Won();
	REQUIRE((wonRes4 && wonRes4.value()));

	res = ReplayFile::FromFile(GetReplay("20210915_180756_PRSC610-Smolensk_35_NE_north_winter.wowsreplay"));  // LOSS
	REQUIRE(res.has_value());
	ReplayFile replay5 = res.value();
	auto wonRes5 = replay5.Won();
	REQUIRE((wonRes5 && !wonRes5.value()));

	/*
	for (auto& entry : fs::recursive_directory_iterator(fs::path("F:\\World_of_Warships_Eu\\replays")))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".wowsreplay")
		{
			std::optional<ReplayFile> res = ReplayFile::FromFile(GetReplay(entry.path().string()));
			if (res.has_value())
			{
				ReplayFile replay = res.value();
				std::cout << entry.path().string() << " -> " << static_cast<int>(replay.WinningTeam()) << std::endl;
			}
		}
	}
	*/
}

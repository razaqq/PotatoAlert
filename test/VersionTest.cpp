// Copyright 2020 <github.com/razaqq>

#include "catch.hpp"
#include "Version.hpp"
#include <vector>


using PotatoAlert::Version;

TEST_CASE( "VersionTest" )
{
	REQUIRE(Version("2.5.9").GetVersionInfo() == std::vector{2, 5, 9});
	REQUIRE(Version("3.7.8.0") == Version("3.7.8.0"));
	REQUIRE(Version("3.7.8.0") == Version("3.7.8"));
	REQUIRE_FALSE(Version("3.7.8.0") < Version("3.7.8"));
	REQUIRE(Version("3.7.9") > Version("3.7.8"));
	REQUIRE(Version("3") < Version("3.7.9"));
	REQUIRE(Version("1.7.9") < Version("3.1"));
	REQUIRE(Version("zzz") < Version("0.0.1"));
	REQUIRE(Version("2.16.0") != Version("3.0.0"));
	REQUIRE_FALSE(Version("3.0.0") < Version("2.16.0"));
}

// Copyright 2020 <github.com/razaqq>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE( "simple" )
{
    std::string toRev = "Hello";

    REQUIRE( toRev == "olleH" );
}

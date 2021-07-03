#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Log.hpp"

TEST_CASE( "Init" )
{
	PotatoAlert::Log::Init();
}

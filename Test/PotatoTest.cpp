#include "Core/Log.hpp"
#include "Core/StandardPaths.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"


struct test_init
{
	test_init()
	{
		PotatoAlert::Core::Log::Init((PotatoAlert::Core::AppDataPath("PotatoAlert") / "PotatoTest.log").string());
	}
} test_init_instance;

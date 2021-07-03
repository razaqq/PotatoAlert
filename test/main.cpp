#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Log.hpp"

struct test_init
{
	test_init()
	{
		PotatoAlert::Log::Init();
	}
} test_init_instance;

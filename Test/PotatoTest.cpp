#include "Core/Log.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"


struct test_init
{
	test_init()
	{
		PotatoAlert::Core::Log::Init();
	}
} test_init_instance;

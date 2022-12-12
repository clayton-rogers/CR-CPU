#include "cast.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <vector>

TEST_CASE("Test properties of cast uint16", "[cast]")
{

	SECTION("At pos limit")
	{
		int a = 0x7FFF;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0x7FFF);
	}
	SECTION("Above limit")
	{
		int a = 0x8000;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0x8000);
	}
	SECTION("At pos max")
	{
		int a = 0xFFFF;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0xFFFF);
	}
	SECTION("Negative")
	{
		int a = -1;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0xFFFF);
	}
	SECTION("At neg max")
	{
		int a = -32768;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0x8000);
	}
}

TEST_CASE("Test cast header", "[cast]")
{

	SECTION("From int")
	{
		int a = 0;
		std::uint16_t target;

		a = 0;
		target = u16(a);
		CHECK(target == 0);

		a = 54355;
		target = u16(a);
		CHECK(target == 54355);

		a = 65535;
		target = u16(a);
		CHECK(target == 65535);

		a = 65536;
		CHECK_THROWS(u16(a));

		a = -1;
		CHECK_THROWS(u16(a));
	}

	SECTION("From uint")
	{
		unsigned int a = 0;

		a = static_cast<unsigned int>(-1);
		CHECK_THROWS(u16(a));

		a = 65535;
		std::uint16_t target = u16(a);
		CHECK(target == 65535);

		a = 65536;
		CHECK_THROWS(u16(a));
	}

	SECTION("Fom size_t")
	{
		std::vector<int> t;
		t.push_back(1);
		t.push_back(2);
		t.push_back(3);
		t.push_back(4);

		std::uint16_t target = u16(t.size());

		CHECK(target == 4);
	}

	SECTION("From char")
	{
		char a;
		std::uint16_t target;

		a = 'A';
		target = u16(a);
		CHECK(target == 0x41);

		a = 'a';
		target = u16(a);
		CHECK(target == 0x61);
	}

	SECTION("From short")
	{
		short a = 0;
		std::uint16_t target = 0;

		a = 0;
		target = u16(a);
		CHECK(target == 0x00);

		a = -1;
		CHECK_THROWS(u16(a));

		a = 32767;
		target = u16(a);
		CHECK(target == 0x7FFF);

		++a;
		CHECK_THROWS(u16(a));
	}
}

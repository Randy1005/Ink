#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>


TEST_CASE("Classify Updated or Removed Pfxt Nodes" * doctest::timeout(300)) {
	ink::Ink ink;

	// e1
	auto& e1 = ink.insert_edge("a", "b", 3);
	// e0
	ink.insert_edge("a", "c", -1);
	// e2
	ink.insert_edge("b", "d", 1);
	// e3
	auto& e3 = ink.insert_edge("b", "e", 2);
	// e4
	ink.insert_edge("d", "c", 3);
	// e5
	ink.insert_edge("c", "f", 1);
	// e6
	auto& e6 = ink.insert_edge("c", "g", 2);
	// e7
	ink.insert_edge("f", "h", -4);
	// e8
	ink.insert_edge("f", "i", 8);
	// e11
	ink.insert_edge("i", "l", 4);
	// e12
	ink.insert_edge("i", "m", 11);
	// e9
	ink.insert_edge("g", "j", 5);
	// e10
	auto& e10 = ink.insert_edge("g", "k", 7);

	ink.report_incsfxt(4, true, false);
	
	// update a few edges
	// e1 -> 9
	// e3 -> 7
	// e6 -> -9
	// e10 -> 11
	ink.insert_edge("a", "b", 9);
	ink.insert_edge("b", "e", 7);
	ink.insert_edge("c", "g", -9);
	ink.insert_edge("g", "k", 11);
	
	auto& b = ink.belongs_to_sfxt;
	
	ink.report_incremental(4, true, false, false);
	REQUIRE(!b[e1.id][0]);
	REQUIRE(!b[e3.id][0]);
	REQUIRE(!b[e10.id][0]);
	REQUIRE(b[e6.id][0]);
}


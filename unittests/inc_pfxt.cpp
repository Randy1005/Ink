#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>
const float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}

void build_chain1(ink::Ink& ink) {
	ink.insert_edge("v1", "v2", 1);
	ink.insert_edge("v2", "v3", 2);
	ink.insert_edge("v3", "v4", 3);
	ink.insert_edge("v4", "v5", 4);
	ink.insert_edge("v5", "v6", -3);
}

void build_chain2(ink::Ink& ink) {
	ink.insert_edge("v7", "v8", -2);
	ink.insert_edge("v8", "v9", -3);
	ink.insert_edge("v9", "v10", -4);
	ink.insert_edge("v10", "v11", -5);
	ink.insert_edge("v11", "v12", -6);
}


void build_bt1(ink::Ink& ink) {
	ink.insert_edge("v1", "v2", 1);
	ink.insert_edge("v1", "v3", 1);
	ink.insert_edge("v2", "v4", 2);
	ink.insert_edge("v2", "v5", 3);
	ink.insert_edge("v3", "v6", 3);
	ink.insert_edge("v3", "v7", 3);
	ink.insert_edge("v6", "v8", 4);
	ink.insert_edge("v6", "v9", 5);
}

void build_bt2(ink::Ink& ink) {
	ink.insert_edge("v10", "v11", -1);
	ink.insert_edge("v10", "v12", -1);
	ink.insert_edge("v11", "v13", -2);
	ink.insert_edge("v11", "v14", -3);
	ink.insert_edge("v12", "v15", -4);
}

void build_bt3(ink::Ink& ink) {
	ink.insert_edge("v16", "v17", 1);
	ink.insert_edge("v17", "v18", -3);
	ink.insert_edge("v18", "v19", 4);
	ink.insert_edge("v18", "v20", 4);
	ink.insert_edge("v20", "v21", -11);
	ink.insert_edge("v20", "v22", -12);
}




TEST_CASE("Update Edges 1" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("v0", "v1", -1);
	ink.insert_edge("v0", "v2", 3);
	ink.insert_edge("v2", "v5", 1);
	ink.insert_edge("v2", "v6", 2);
	ink.insert_edge("v5", "v1", 1);
	ink.insert_edge("v1", "v3", 1);
	ink.insert_edge("v1", "v4", 2);
	ink.insert_edge("v3", "v7", -4);
	ink.insert_edge("v3", "v8", 8);
	
	// report once with incsfxt, save the pfxt nodes
	auto paths = ink.report_incsfxt(10, true);
	REQUIRE(paths.size() == 7);

	// modify v0 -> v2 weight
	// v0->v2 is still a prefix tree edge
	// enable use_leaders, enable save_pfxt_nodes
	ink.insert_edge("v0", "v2", 5);
	paths = ink.report_incremental(10, true, true);
	
	REQUIRE(paths.size() == 7);
	REQUIRE(float_equal(paths[0].weight, -4));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 7));
	REQUIRE(float_equal(paths[4].weight, 8));
	REQUIRE(float_equal(paths[5].weight, 9));
	REQUIRE(float_equal(paths[6].weight, 16));

	// modify v0 -> v2 weight
	// v0->v2 became a suffix tree edge
	ink.insert_edge("v0", "v2", -4);
	paths = ink.report_incremental(10, true, true);
	REQUIRE(paths.size() == 7);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -2));
	REQUIRE(float_equal(paths[3].weight, 0));
	REQUIRE(float_equal(paths[4].weight, 1));
	REQUIRE(float_equal(paths[5].weight, 7));
	REQUIRE(float_equal(paths[6].weight, 8));
}


TEST_CASE("Update Edges 2" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("A", "B", -1);
	ink.insert_edge("A", "C", 3);
	ink.insert_edge("C", "D", 1);
	ink.insert_edge("C", "E", 2);
	ink.insert_edge("D", "B", 3);

	ink.insert_edge("B", "F", 1);
	ink.insert_edge("B", "G", 2);
	ink.insert_edge("F", "H", -4);
	ink.insert_edge("F", "I", 8);
	ink.insert_edge("I", "L", 4);
	ink.insert_edge("I", "M", 11);

	ink.insert_edge("G", "J", 5);
	ink.insert_edge("G", "K", 7);

	auto paths = ink.report_incsfxt(20, true);
	REQUIRE(paths.size() == 11);

	// modify A->C's weight to 0, A->C remains a prefix tree edge
	ink.insert_edge("A", "C", 0);
	paths = ink.report_incremental(20, true, true);
	REQUIRE(paths.size() == 11);
	REQUIRE(float_equal(paths[0].weight, -4));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 2));
	REQUIRE(float_equal(paths[3].weight, 6));
	REQUIRE(float_equal(paths[4].weight, 8));
	REQUIRE(float_equal(paths[5].weight, 11));
	REQUIRE(float_equal(paths[6].weight, 12));
	REQUIRE(float_equal(paths[7].weight, 13));
	REQUIRE(float_equal(paths[8].weight, 17));
	REQUIRE(float_equal(paths[9].weight, 19));
	REQUIRE(float_equal(paths[10].weight, 24));


	paths = ink.report_incsfxt(20, true);
	
	// modify A->B's weight to 5, A->C becomes a suffix tree edge
	ink.insert_edge("A", "B", 5);
	paths = ink.report_incremental(20, true, true);
	REQUIRE(paths.size() == 11);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 2));
	REQUIRE(float_equal(paths[2].weight, 2));
	REQUIRE(float_equal(paths[3].weight, 11));
	REQUIRE(float_equal(paths[4].weight, 12));
	REQUIRE(float_equal(paths[5].weight, 13));
	REQUIRE(float_equal(paths[6].weight, 14));
	REQUIRE(float_equal(paths[7].weight, 17));
	REQUIRE(float_equal(paths[8].weight, 18));
	REQUIRE(float_equal(paths[9].weight, 24));
	REQUIRE(float_equal(paths[10].weight, 25));

	// if we report less paths
	// the paths should not be affected
	//paths = ink.report_incremental(7, true, true);
	//REQUIRE(float_equal(paths[0].weight, 1));
	//REQUIRE(float_equal(paths[1].weight, 2));
	//REQUIRE(float_equal(paths[2].weight, 2));
	//REQUIRE(float_equal(paths[3].weight, 11));
	//REQUIRE(float_equal(paths[4].weight, 12));
	//REQUIRE(float_equal(paths[5].weight, 13));
	//REQUIRE(float_equal(paths[6].weight, 14));
}


TEST_CASE("Update Edges 3" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("S", "A", 0);
	ink.insert_edge("S", "B", 0);
	ink.insert_edge("S", "C", 0);

	ink.insert_edge("A", "D", 0);
	ink.insert_edge("A", "E", 1);
	ink.insert_edge("A", "F", 2);

	ink.insert_edge("D", "K", -1);
	ink.insert_edge("D", "L", 4);
	ink.insert_edge("F", "M", 3);
	ink.insert_edge("F", "N", 1);

	ink.insert_edge("B", "F", -1);
	ink.insert_edge("B", "G", 8);
	ink.insert_edge("B", "H", 1);

	ink.insert_edge("H", "O", 7);
	ink.insert_edge("H", "P", 2);

	ink.insert_edge("C", "H", 2);
	ink.insert_edge("C", "I", 3);
	ink.insert_edge("C", "J", 4);

	ink.insert_edge("J", "Q", 4);
	ink.insert_edge("J", "R", -3);

	// report first with incsfxt
	// and save the pfxt nodes
	auto paths = ink.report_incsfxt(20, true);
	REQUIRE(paths.size() == 15);

	// remove edge S->B, modify edge S->C
	ink.remove_edge("S", "B");
	ink.insert_edge("S", "C", 5);
	
	// pfxt node SC remains a pfxt node, record as leader
	// pfxt node BH, BG, FM would be recorded as leader
	paths = ink.report_incremental(20, true, true);
	REQUIRE(paths.size() == 15);
	REQUIRE(float_equal(paths[0].weight, -1));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 1));
	REQUIRE(float_equal(paths[3].weight, 2));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 5));
	REQUIRE(float_equal(paths[8].weight, 6));
	REQUIRE(float_equal(paths[9].weight, 8));
	REQUIRE(float_equal(paths[10].weight, 8));
	REQUIRE(float_equal(paths[11].weight, 8));
	REQUIRE(float_equal(paths[12].weight, 9));
	REQUIRE(float_equal(paths[13].weight, 13));
	REQUIRE(float_equal(paths[14].weight, 14));
}




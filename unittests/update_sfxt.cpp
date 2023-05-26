#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>
const float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}

TEST_CASE("update sfxt test 1" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("v8", "v2", 1);
	
	auto paths = ink.report_global(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 1));
	
	ink.insert_edge("v9", "v2", 1);

	paths = ink.report_global(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 1));
	
	ink.insert_edge("v0", "v3", 1);
	ink.insert_edge("v1", "v3", 1);
	ink.insert_edge("v2", "v3", 1);
	paths = ink.report_global(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 2));
	REQUIRE(float_equal(paths[3].weight, 2));
	
	ink.insert_edge("v3", "v5", 1);
	ink.insert_edge("v3", "v4", 1);
	paths = ink.report_global(10);
	REQUIRE(paths.size() == 8);
	REQUIRE(float_equal(paths[0].weight, 2));
	REQUIRE(float_equal(paths[1].weight, 2));
	REQUIRE(float_equal(paths[2].weight, 2));
	REQUIRE(float_equal(paths[3].weight, 2));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 3));
	REQUIRE(float_equal(paths[7].weight, 3));

	// update v2 -> v3
	ink.insert_edge("v2", "v3", -2);
	paths = ink.report_global(10);
	REQUIRE(paths.size() == 8);
	REQUIRE(float_equal(paths[0].weight, 0));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 0));
	REQUIRE(float_equal(paths[3].weight, 0));
	REQUIRE(float_equal(paths[4].weight, 2));
	REQUIRE(float_equal(paths[5].weight, 2));
	REQUIRE(float_equal(paths[6].weight, 2));
	REQUIRE(float_equal(paths[7].weight, 2));

	// insert new edge v9 -> v10
	ink.insert_edge("v9", "v10", -3);
	paths = ink.report_global(10);
	REQUIRE(paths.size() == 9);
	REQUIRE(float_equal(paths[0].weight, -3));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 0));
	REQUIRE(float_equal(paths[3].weight, 0));
	REQUIRE(float_equal(paths[4].weight, 0));
	REQUIRE(float_equal(paths[5].weight, 2));
	REQUIRE(float_equal(paths[6].weight, 2));
	REQUIRE(float_equal(paths[7].weight, 2));
	REQUIRE(float_equal(paths[8].weight, 2));
	
	// remove edge v3 -> v5
	ink.remove_edge("v2", "v3");
	paths = ink.report_global(10);
	
	REQUIRE(paths.size() == 7);
	REQUIRE(float_equal(paths[0].weight, -3));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 1));
	REQUIRE(float_equal(paths[3].weight, 2));
	REQUIRE(float_equal(paths[4].weight, 2));
	REQUIRE(float_equal(paths[5].weight, 2));
	REQUIRE(float_equal(paths[6].weight, 2));
	
}

TEST_CASE("update sfxt test 2" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("A", "B",
		0, std::nullopt, std::nullopt, 0,
		0, std::nullopt, std::nullopt, 0);

	auto paths = ink.report_global(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 0));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 0));
	REQUIRE(float_equal(paths[3].weight, 0));


	ink.insert_edge("C", "D",
		0, std::nullopt, std::nullopt, 0,
		0, std::nullopt, std::nullopt, 0);
	paths = ink.report_global(10);
	REQUIRE(paths.size() == 8);
	REQUIRE(float_equal(paths[0].weight, 0));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 0));
	REQUIRE(float_equal(paths[3].weight, 0));
	REQUIRE(float_equal(paths[4].weight, 0));
	REQUIRE(float_equal(paths[5].weight, 0));
	REQUIRE(float_equal(paths[6].weight, 0));
	REQUIRE(float_equal(paths[7].weight, 0));
	
	ink.insert_edge("E", "F",
		0, std::nullopt, std::nullopt, 0,
		0, std::nullopt, std::nullopt, 0);
	paths = ink.report_global(20);
	REQUIRE(paths.size() == 12);
	REQUIRE(float_equal(paths[0].weight, 0));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 0));
	REQUIRE(float_equal(paths[3].weight, 0));
	REQUIRE(float_equal(paths[4].weight, 0));
	REQUIRE(float_equal(paths[5].weight, 0));
	REQUIRE(float_equal(paths[6].weight, 0));
	REQUIRE(float_equal(paths[7].weight, 0));
	REQUIRE(float_equal(paths[8].weight, 0));
	REQUIRE(float_equal(paths[9].weight, 0));
	REQUIRE(float_equal(paths[10].weight, 0));
	REQUIRE(float_equal(paths[11].weight, 0));
	
	ink.insert_edge("D", "E", 
		std::nullopt, std::nullopt, std::nullopt, std::nullopt, 
		std::nullopt, 1, 2, std::nullopt);
	paths = ink.report_global(20);
	REQUIRE(paths.size() == 20);
	REQUIRE(float_equal(paths[0].weight, 0));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 0));
	REQUIRE(float_equal(paths[3].weight, 0));
	REQUIRE(float_equal(paths[4].weight, 1));
	REQUIRE(float_equal(paths[5].weight, 1));
	REQUIRE(float_equal(paths[6].weight, 1));
	REQUIRE(float_equal(paths[7].weight, 1));
	REQUIRE(float_equal(paths[8].weight, 1));
	REQUIRE(float_equal(paths[9].weight, 1));
	REQUIRE(float_equal(paths[10].weight, 1));
	REQUIRE(float_equal(paths[11].weight, 1));
	REQUIRE(float_equal(paths[12].weight, 1));
	REQUIRE(float_equal(paths[13].weight, 1));
	REQUIRE(float_equal(paths[14].weight, 1));
	REQUIRE(float_equal(paths[15].weight, 1));
	REQUIRE(float_equal(paths[16].weight, 1));
	REQUIRE(float_equal(paths[17].weight, 1));
	REQUIRE(float_equal(paths[18].weight, 1));
	REQUIRE(float_equal(paths[19].weight, 1));
	
	
	ink.insert_edge("B", "E", 
		std::nullopt, 3, 4, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	paths = ink.report_global(20);
	REQUIRE(paths.size() == 20);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 1));
	REQUIRE(float_equal(paths[3].weight, 1));
	REQUIRE(float_equal(paths[4].weight, 1));
	REQUIRE(float_equal(paths[5].weight, 1));
	REQUIRE(float_equal(paths[6].weight, 1));
	REQUIRE(float_equal(paths[7].weight, 1));
	REQUIRE(float_equal(paths[8].weight, 1));
	REQUIRE(float_equal(paths[9].weight, 1));
	REQUIRE(float_equal(paths[10].weight, 1));
	REQUIRE(float_equal(paths[11].weight, 1));
	REQUIRE(float_equal(paths[12].weight, 1));
	REQUIRE(float_equal(paths[13].weight, 1));
	REQUIRE(float_equal(paths[14].weight, 1));
	REQUIRE(float_equal(paths[15].weight, 1));
	REQUIRE(float_equal(paths[16].weight, 2));
	REQUIRE(float_equal(paths[17].weight, 2));
	REQUIRE(float_equal(paths[18].weight, 2));
	REQUIRE(float_equal(paths[19].weight, 2));
	
	ink.insert_edge("D", "E", 
		std::nullopt, 6, 7, std::nullopt, 
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	paths = ink.report_global(20);
	REQUIRE(paths.size() == 20);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 3));
	REQUIRE(float_equal(paths[2].weight, 3));
	REQUIRE(float_equal(paths[3].weight, 3));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 3));
	REQUIRE(float_equal(paths[7].weight, 3));
	REQUIRE(float_equal(paths[8].weight, 3));
	REQUIRE(float_equal(paths[9].weight, 3));
	REQUIRE(float_equal(paths[10].weight, 3));
	REQUIRE(float_equal(paths[11].weight, 3));
	REQUIRE(float_equal(paths[12].weight, 3));
	REQUIRE(float_equal(paths[13].weight, 3));
	REQUIRE(float_equal(paths[14].weight, 3));
	REQUIRE(float_equal(paths[15].weight, 3));
	REQUIRE(float_equal(paths[16].weight, 4));
	REQUIRE(float_equal(paths[17].weight, 4));
	REQUIRE(float_equal(paths[18].weight, 4));
	REQUIRE(float_equal(paths[19].weight, 4));
	
	ink.insert_edge("G", "H", 0, 0, 0, 0);
	paths = ink.report_global(20);
	
	REQUIRE(paths.size() == 20);
	REQUIRE(float_equal(paths[0].weight, 0));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 0));
	REQUIRE(float_equal(paths[3].weight, 0));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 3));
	REQUIRE(float_equal(paths[7].weight, 3));
	REQUIRE(float_equal(paths[8].weight, 3));
	REQUIRE(float_equal(paths[9].weight, 3));
	REQUIRE(float_equal(paths[10].weight, 3));
	REQUIRE(float_equal(paths[11].weight, 3));
	REQUIRE(float_equal(paths[12].weight, 3));
	REQUIRE(float_equal(paths[13].weight, 3));
	REQUIRE(float_equal(paths[14].weight, 3));
	REQUIRE(float_equal(paths[15].weight, 3));
	REQUIRE(float_equal(paths[16].weight, 3));
	REQUIRE(float_equal(paths[17].weight, 3));
	REQUIRE(float_equal(paths[18].weight, 3));
	REQUIRE(float_equal(paths[19].weight, 3));

}


TEST_CASE("update sfxt test 3" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("v0", "v2", 1);
	ink.insert_edge("v1", "v2", 1);
	ink.insert_edge("v2", "v3", -5);
	ink.insert_edge("v2", "v4", 1);
	ink.insert_edge("v4", "v5", 0);
	auto paths = ink.report_global(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, -4));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, 2));
	REQUIRE(float_equal(paths[3].weight, 2));


	ink.insert_edge("v1", "v2", -1);
	ink.insert_edge("v4", "v5", -1);

	paths = ink.report_global(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, -6));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -1));
	REQUIRE(float_equal(paths[3].weight, 1));
}


TEST_CASE("update sfxt test 4" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("A", "B", 0);
	ink.insert_edge("B", "C", 1);
	ink.insert_edge("C", "D", 0);

	ink.insert_edge("A", "E", 0);
	ink.insert_edge("F", "G", 0);

	auto paths = ink.report_global(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 0));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 1));

	ink.insert_edge("E", "F", 5);
	ink.insert_edge("E", "H", 10);
	ink.insert_edge("E", "I", 15);
	paths = ink.report_global(10);

	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 5));
	REQUIRE(float_equal(paths[2].weight, 10));
	REQUIRE(float_equal(paths[3].weight, 15));

}

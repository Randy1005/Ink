#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>
const float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}




TEST_CASE("Single Source Tree" * doctest::timeout(300)) {
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
	
	auto paths = ink.report_incremental(10);
	REQUIRE(paths.size() == 7);

	ink.dump_pfxt(std::cout);

}




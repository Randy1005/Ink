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


TEST_CASE("1 Chain" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("v1", "v2", 1);
	REQUIRE(ink.num_verts() == 2);
	REQUIRE(ink.num_edges() == 1);
	auto paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 1));

	ink.insert_edge("v2", "v3", 2);
	REQUIRE(ink.num_verts() == 3);
	REQUIRE(ink.num_edges() == 2);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 3));


	ink.insert_edge("v3", "v4", 3);
	REQUIRE(ink.num_verts() == 4);
	REQUIRE(ink.num_edges() == 3);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 6));

	ink.insert_edge("v4", "v5", 4);
	REQUIRE(ink.num_verts() == 5);
	REQUIRE(ink.num_edges() == 4);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 10));

	ink.insert_edge("v5", "v6", -3);
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 5);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 7));

	// incremental destruction from back of chain
	ink.remove_edge("v5", "v6");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 4);
	// v6 is now dangling, ink will not report a path with only 1 vertex
	// ink will also not remove this dangling vertex, leaving it up to
	// the user to handle it
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 10);


	ink.remove_edge("v4", "v5");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 3);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 6);

	
	ink.remove_edge("v3", "v4");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 2);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 3);

	ink.remove_edge("v2", "v3");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 1);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 1);

	ink.remove_edge("v1", "v2");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 0);
	paths = ink.report(10);
	REQUIRE(paths.size() == 0);

	ink.remove_vertex("v1");
	ink.remove_vertex("v2");
	ink.remove_vertex("v3");
	ink.remove_vertex("v4");
	ink.remove_vertex("v5");
	ink.remove_vertex("v6");
	REQUIRE(ink.num_verts() == 0);
}

TEST_CASE("2 chains" * doctest::timeout(300)) {
	ink::Ink ink;
	build_chain1(ink);
		
	ink.insert_edge("v7", "v8", -2);
	REQUIRE(ink.num_verts() == 8);
	REQUIRE(ink.num_edges() == 6);
	auto paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -2));
	REQUIRE(float_equal(paths[1].weight, 7));

	ink.insert_edge("v8", "v9", -4);
	REQUIRE(ink.num_verts() == 9);
	REQUIRE(ink.num_edges() == 7);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -6));
	REQUIRE(float_equal(paths[1].weight, 7));

	ink.insert_edge("v9", "v10", -5);
	REQUIRE(ink.num_verts() == 10);
	REQUIRE(ink.num_edges() == 8);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -11));
	REQUIRE(float_equal(paths[1].weight, 7));

	ink.insert_edge("v10", "v11", -6);
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 9);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -17));
	REQUIRE(float_equal(paths[1].weight, 7));

	// incremental destruction from back of 2 chains
	ink.remove_edge("v5", "v6");
	ink.remove_edge("v10", "v11");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 7);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -11));
	REQUIRE(float_equal(paths[1].weight, 10));

	ink.remove_edge("v4", "v5");
	ink.remove_edge("v9", "v10");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 5);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -6));
	REQUIRE(float_equal(paths[1].weight, 6));

	ink.remove_edge("v3", "v4");
	ink.remove_edge("v8", "v9");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 3);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -2));
	REQUIRE(float_equal(paths[1].weight, 3));

	ink.remove_edge("v2", "v3");
	ink.remove_edge("v7", "v8");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 1);
	paths = ink.report(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 1));

	ink.remove_edge("v1", "v2");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 0);
	paths = ink.report(10);
	REQUIRE(paths.size() == 0);
}

TEST_CASE("3 chains" * doctest::timeout(300)) {
	ink::Ink ink;
	build_chain1(ink);
	build_chain2(ink);	

	ink.insert_edge("v13", "v14", 0);
	REQUIRE(ink.num_verts() == 14);
	REQUIRE(ink.num_edges() == 11);
	auto paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -20));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 7));

	ink.insert_edge("v14", "v15", -2);
	REQUIRE(ink.num_verts() == 15);
	REQUIRE(ink.num_edges() == 12);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -20));
	REQUIRE(float_equal(paths[1].weight, -2));
	REQUIRE(float_equal(paths[2].weight, 7));

	ink.insert_edge("v15", "v16", 3);
	REQUIRE(ink.num_verts() == 16);
	REQUIRE(ink.num_edges() == 13);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -20));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 7));

	
	ink.insert_edge("v16", "v17", 13.5);
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 14);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -20));
	REQUIRE(float_equal(paths[1].weight, 7));
	REQUIRE(float_equal(paths[2].weight, 14.5));

	// incremental destruction of 3 chains
	ink.remove_edge("v16", "v17");
	ink.remove_edge("v11", "v12");
	ink.remove_edge("v5", "v6");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 11);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -14));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 10));
	
	ink.remove_edge("v15", "v16");
	ink.remove_edge("v10", "v11");
	ink.remove_edge("v4", "v5");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 8);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -9));
	REQUIRE(float_equal(paths[1].weight, -2));
	REQUIRE(float_equal(paths[2].weight, 6));

	ink.remove_edge("v14", "v15");
	ink.remove_edge("v9", "v10");
	ink.remove_edge("v3", "v4");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 5);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 3));

	ink.remove_edge("v13", "v14");
	ink.remove_edge("v8", "v9");
	ink.remove_edge("v2", "v3");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 2);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -2));
	REQUIRE(float_equal(paths[1].weight, 1));

	ink.remove_edge("v7", "v8");
	ink.remove_edge("v1", "v2");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 0);
	paths = ink.report(10);
	REQUIRE(paths.size() == 0);
}


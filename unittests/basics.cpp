#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>
const float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}


TEST_CASE("Vertex Insertion + Removal" * doctest::timeout(300)) {
	ink::Ink ink;

	auto& v1 = ink.insert_vertex("v1");
	REQUIRE(v1.name == "v1");
	REQUIRE(ink.num_verts() == 1);

	// Duplicate insertion: should do nothing
	ink.insert_vertex("v1");
	REQUIRE(v1.name == "v1");
	REQUIRE(ink.num_verts() == 1);

	auto& v2 = ink.insert_vertex("v2");
	REQUIRE(v2.name == "v2");
	REQUIRE(ink.num_verts() == 2);

	ink.insert_edge("v1", "v2",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);
	
	ink.insert_edge("v1", "v3",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);
	
	ink.insert_edge("v1", "v4",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);

	ink.insert_edge("v1", "v5",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);
	
	ink.insert_edge("v6", "v1",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);

	ink.insert_edge("v7", "v1",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);


	ink.insert_edge("v2", "v8",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);

	
	ink.insert_edge("v2", "v9",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);
	

	REQUIRE(ink.num_edges() == 8);
	REQUIRE(ink.num_verts() == 9);
	REQUIRE(v1.num_fanouts() == 4);
	REQUIRE(v1.num_fanins() == 2);
	REQUIRE(v2.num_fanouts() == 2);
	REQUIRE(v2.num_fanins() == 1);

	// remove v1
	// fanout to v2, v3, v4, v5
	// fanin from v6, v7
	// should all get removed

	ink.remove_vertex("v1");
	
	// NOTE:
	// possible issue
	// I left v1's ref exposed here
	// but the information of v1 is no longer
	// valid, but the ref is still accessible
	
	
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(ink.num_verts() == 8);
	REQUIRE(v2.num_fanouts() == 2);
	REQUIRE(v2.num_fanins() == 0);

	ink.remove_vertex("v2");
	REQUIRE(ink.num_edges() == 0);
	REQUIRE(ink.num_verts() == 7);

}

TEST_CASE("Edge Insertion + Removal" * doctest::timeout(300)) {
	ink::Ink ink;

	auto& e12 = ink.insert_edge("v1", "v2",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);

	REQUIRE(ink.num_edges() == 1);
	REQUIRE(e12.from.num_fanouts() == 1);
	REQUIRE(e12.to.num_fanins() == 1);

	ink.insert_edge("v1", "v2",
		0, 0, 3.89, 0,
		0, 1.5, 2.8, 0);


	REQUIRE(ink.num_edges() == 1);

	ink.insert_edge("v1", "v3",
		0, 0, 3.89, 2.3,
		0, 1.5, 2.8, 3.89);

	auto& v1 = ink.insert_vertex("v1");
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(v1.num_fanouts() == 2);

	// this will overwrite all weight values
	auto& e13 = ink.insert_edge("v1", "v3",
		0, 0, 3.89, 2.3,
		0, 1.5, 2.8, 3.89);
	REQUIRE(float_equal(*e13.weights[0], 0));
	REQUIRE(float_equal(*e13.weights[1], 0));
	REQUIRE(float_equal(*e13.weights[2], 3.89));
	REQUIRE(float_equal(*e13.weights[3], 2.3));
	REQUIRE(float_equal(*e13.weights[4], 0));
	REQUIRE(float_equal(*e13.weights[5], 1.5));
	REQUIRE(float_equal(*e13.weights[6], 2.8));
	REQUIRE(float_equal(*e13.weights[7], 3.89));
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(v1.num_fanouts() == 2);

	auto& e35 = ink.insert_edge("v3", "v5",
		0, 0, 3.89, 0,
		0, 1.5, 2.8, 0);

	REQUIRE(ink.num_edges() == 3);
	REQUIRE(e35.from.num_fanouts() == 1);
	REQUIRE(e35.to.num_fanins() == 1);


	ink.remove_edge("v1", "v2");
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(v1.num_fanouts() == 1);

	ink.remove_edge("v1", "v3");
	REQUIRE(ink.num_edges() == 1);
	REQUIRE(v1.num_fanouts() == 0);

	ink.remove_edge("v3", "v5");	
	REQUIRE(ink.num_edges() == 0);
}


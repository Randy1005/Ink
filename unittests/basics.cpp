#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

TEST_CASE("Vertex Insertion + Removal" * doctest::timeout(300)) {
	ink::Ink ink;

	ink::Vert& v = ink.insert_vertex("v1");
	REQUIRE(v.name == "v1");
	REQUIRE(ink.num_verts() == 1);
	
	v = ink.insert_vertex("v1");
	REQUIRE(v.name == "v1");
	REQUIRE(ink.num_verts() == 1);

	v = ink.insert_vertex("v2");
	REQUIRE(v.name == "v2");
	REQUIRE(ink.num_verts() == 2);
}

TEST_CASE("Edge Insertion + Removal" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("v1", "v2",
		0, 0, std::nullopt, 0,
		0, 1.5, 2.8, 0);

	REQUIRE(ink.num_edges() == 1);

	ink.insert_edge("v1", "v2",
		0, 0, 3.89, 0,
		0, 1.5, 2.8, 0);


	REQUIRE(ink.num_edges() == 1);

	ink.insert_edge("v1", "v3",
		0, 0, 3.89, 2.3,
		0, 1.5, 2.8, 3.89);

	REQUIRE(ink.num_edges() == 2);
}


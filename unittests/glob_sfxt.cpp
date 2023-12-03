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


// TODO: rewrite these unittests, I removed the path.size > 1 check in
// report_incsfxt
TEST_CASE("1 Chain (using global sfxt)" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("v1", "v2", 1);
	REQUIRE(ink.num_verts() == 2);
	REQUIRE(ink.num_edges() == 1);
	auto paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 1));

	ink.insert_edge("v2", "v3", 2);
	REQUIRE(ink.num_verts() == 3);
	REQUIRE(ink.num_edges() == 2);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 3));


	ink.insert_edge("v3", "v4", 3);
	REQUIRE(ink.num_verts() == 4);
	REQUIRE(ink.num_edges() == 3);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 6));

	ink.insert_edge("v4", "v5", 4);
	REQUIRE(ink.num_verts() == 5);
	REQUIRE(ink.num_edges() == 4);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 10));

	ink.insert_edge("v5", "v6", -3);
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 5);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 7));

	// incremental destruction from back of chain
	ink.remove_edge("v5", "v6");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 4);
	// v6 is now dangling, ink will not report_incsfxt a path with only 1 vertex
	// ink will also not remove this dangling vertex, leaving it up to
	// the user to handle it
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 10);


	ink.remove_edge("v4", "v5");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 3);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 6);

	
	ink.remove_edge("v3", "v4");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 2);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 3);

	ink.remove_edge("v2", "v3");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 1);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(paths[0].weight == 1);

	ink.remove_edge("v1", "v2");
	REQUIRE(ink.num_verts() == 6);
	REQUIRE(ink.num_edges() == 0);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 0);

	ink.remove_vertex("v1");
	ink.remove_vertex("v2");
	ink.remove_vertex("v3");
	ink.remove_vertex("v4");
	ink.remove_vertex("v5");
	ink.remove_vertex("v6");
	REQUIRE(ink.num_verts() == 0);
}

TEST_CASE("2 chains (using global sfxt)" * doctest::timeout(300)) {
	ink::Ink ink;
	build_chain1(ink);
		
	ink.insert_edge("v7", "v8", -2);
	REQUIRE(ink.num_verts() == 8);
	REQUIRE(ink.num_edges() == 6);
	auto paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -2));
	REQUIRE(float_equal(paths[1].weight, 7));

	ink.insert_edge("v8", "v9", -4);
	REQUIRE(ink.num_verts() == 9);
	REQUIRE(ink.num_edges() == 7);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -6));
	REQUIRE(float_equal(paths[1].weight, 7));

	ink.insert_edge("v9", "v10", -5);
	REQUIRE(ink.num_verts() == 10);
	REQUIRE(ink.num_edges() == 8);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -11));
	REQUIRE(float_equal(paths[1].weight, 7));

	ink.insert_edge("v10", "v11", -6);
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -17));
	REQUIRE(float_equal(paths[1].weight, 7));

	// incremental destruction from back of 2 chains
	ink.remove_edge("v5", "v6");
	ink.remove_edge("v10", "v11");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 7);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -11));
	REQUIRE(float_equal(paths[1].weight, 10));

	ink.remove_edge("v4", "v5");
	ink.remove_edge("v9", "v10");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 5);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -6));
	REQUIRE(float_equal(paths[1].weight, 6));

	ink.remove_edge("v3", "v4");
	ink.remove_edge("v8", "v9");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 3);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -2));
	REQUIRE(float_equal(paths[1].weight, 3));

	ink.remove_edge("v2", "v3");
	ink.remove_edge("v7", "v8");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 1);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 1);
	REQUIRE(float_equal(paths[0].weight, 1));

	ink.remove_edge("v1", "v2");
	REQUIRE(ink.num_verts() == 11);
	REQUIRE(ink.num_edges() == 0);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 0);
}

TEST_CASE("3 chains (using global sfxt)" * doctest::timeout(50000000)) {
	ink::Ink ink;
	build_chain1(ink);
	build_chain2(ink);	

	ink.insert_edge("v13", "v14", 0);
	REQUIRE(ink.num_verts() == 14);
	REQUIRE(ink.num_edges() == 11);
	auto paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -20));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 7));

	ink.insert_edge("v14", "v15", -2);
	REQUIRE(ink.num_verts() == 15);
	REQUIRE(ink.num_edges() == 12);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -20));
	REQUIRE(float_equal(paths[1].weight, -2));
	REQUIRE(float_equal(paths[2].weight, 7));

	ink.insert_edge("v15", "v16", 3);
	REQUIRE(ink.num_verts() == 16);
	REQUIRE(ink.num_edges() == 13);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -20));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 7));

	
	ink.insert_edge("v16", "v17", 13.5);
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 14);
	paths = ink.report_incsfxt(10);
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
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -14));
	REQUIRE(float_equal(paths[1].weight, 1));
	REQUIRE(float_equal(paths[2].weight, 10));
	
	ink.remove_edge("v15", "v16");
	ink.remove_edge("v10", "v11");
	ink.remove_edge("v4", "v5");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 8);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -9));
	REQUIRE(float_equal(paths[1].weight, -2));
	REQUIRE(float_equal(paths[2].weight, 6));


	ink.remove_edge("v14", "v15");
	ink.remove_edge("v9", "v10");
	ink.remove_edge("v3", "v4");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 5);
	
	paths = ink.report_incsfxt(10);

	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, 0));
	REQUIRE(float_equal(paths[2].weight, 3));

	ink.remove_edge("v13", "v14");
	ink.remove_edge("v8", "v9");
	ink.remove_edge("v2", "v3");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 2);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, -2));
	REQUIRE(float_equal(paths[1].weight, 1));

	ink.remove_edge("v7", "v8");
	ink.remove_edge("v1", "v2");
	REQUIRE(ink.num_verts() == 17);
	REQUIRE(ink.num_edges() == 0);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 0);
}

TEST_CASE("1 Binary Tree (using global sfxt)" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("v1", "v2", 1);
	ink.insert_edge("v1", "v3", 1);
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(ink.num_verts() == 3);
	auto paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 1));
	
	ink.insert_edge("v2", "v4", 2);
	REQUIRE(ink.num_edges() == 3);
	REQUIRE(ink.num_verts() == 4);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	
	ink.insert_edge("v2", "v5", 3);
	REQUIRE(ink.num_edges() == 4);
	REQUIRE(ink.num_verts() == 5);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	REQUIRE(float_equal(paths[2].weight, 4));
	

	ink.insert_edge("v3", "v6", 3);
	REQUIRE(ink.num_edges() == 5);
	REQUIRE(ink.num_verts() == 6);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	
	
	ink.insert_edge("v3", "v7", 3);
	REQUIRE(ink.num_edges() == 6);
	REQUIRE(ink.num_verts() == 7);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 4));
	
	ink.insert_edge("v6", "v8", 4);
	REQUIRE(ink.num_edges() == 7);
	REQUIRE(ink.num_verts() == 8);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 8));
	
	
	ink.insert_edge("v6", "v9", 5);
	REQUIRE(ink.num_edges() == 8);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 5);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 8));
	REQUIRE(float_equal(paths[4].weight, 9));

	// incremental destruction of 1 binary tree
	ink.remove_edge("v6", "v9");
	REQUIRE(ink.num_edges() == 7);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 8));
	
	ink.remove_edge("v6", "v8");
	REQUIRE(ink.num_edges() == 6);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 4));


	ink.remove_edge("v3", "v7");
	REQUIRE(ink.num_edges() == 5);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));

	ink.remove_edge("v3", "v6");
	REQUIRE(ink.num_edges() == 4);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	REQUIRE(float_equal(paths[2].weight, 4));

	ink.remove_edge("v2", "v5");
	REQUIRE(ink.num_edges() == 3);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	
	ink.remove_edge("v2", "v4");
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 1));
	
	ink.remove_edge("v1", "v2");
	ink.remove_edge("v1", "v3");
	REQUIRE(ink.num_edges() == 0);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report_incsfxt(10);
	REQUIRE(paths.size() == 0);

}

TEST_CASE("2 Binary Trees (using global sfxt)" * doctest::timeout(300)) {
	ink::Ink ink;

	build_bt1(ink);

	ink.insert_edge("v10", "v11", -1);
	REQUIRE(ink.num_edges() == 9);
	REQUIRE(ink.num_verts() == 11);
	auto paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 6);
	REQUIRE(float_equal(paths[0].weight, -1));
	REQUIRE(float_equal(paths[1].weight, 3));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 4));
	REQUIRE(float_equal(paths[4].weight, 8));
	REQUIRE(float_equal(paths[5].weight, 9));
	
	ink.insert_edge("v10", "v12", -1);
	REQUIRE(ink.num_edges() == 10);
	REQUIRE(ink.num_verts() == 12);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 7);
	REQUIRE(float_equal(paths[0].weight, -1));
	REQUIRE(float_equal(paths[1].weight, -1));
	REQUIRE(float_equal(paths[2].weight, 3));
	REQUIRE(float_equal(paths[3].weight, 4));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 8));
	REQUIRE(float_equal(paths[6].weight, 9));
	
	
	
	ink.insert_edge("v11", "v13", -2);
	REQUIRE(ink.num_edges() == 11);
	REQUIRE(ink.num_verts() == 13);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 7);
	REQUIRE(float_equal(paths[0].weight, -3));
	REQUIRE(float_equal(paths[1].weight, -1));
	REQUIRE(float_equal(paths[2].weight, 3));
	REQUIRE(float_equal(paths[3].weight, 4));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 8));
	REQUIRE(float_equal(paths[6].weight, 9));
	
	
	ink.insert_edge("v11", "v14", -3);
	REQUIRE(ink.num_edges() == 12);
	REQUIRE(ink.num_verts() == 14);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 8);
	REQUIRE(float_equal(paths[0].weight, -4));
	REQUIRE(float_equal(paths[1].weight, -3));
	REQUIRE(float_equal(paths[2].weight, -1));
	REQUIRE(float_equal(paths[3].weight, 3));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 8));
	REQUIRE(float_equal(paths[7].weight, 9));
	
	
	ink.insert_edge("v12", "v15", -4);
	REQUIRE(ink.num_edges() == 13);
	REQUIRE(ink.num_verts() == 15);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 8);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 3));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 8));
	REQUIRE(float_equal(paths[7].weight, 9));

	// incremental destruction of binary tree 2
	ink.remove_edge("v12", "v15");
	REQUIRE(ink.num_edges() == 12);
	REQUIRE(ink.num_verts() == 15);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 8);
	REQUIRE(float_equal(paths[0].weight, -4));
	REQUIRE(float_equal(paths[1].weight, -3));
	REQUIRE(float_equal(paths[2].weight, -1));
	REQUIRE(float_equal(paths[3].weight, 3));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 8));
	REQUIRE(float_equal(paths[7].weight, 9));

	ink.remove_edge("v11", "v14");
	REQUIRE(ink.num_edges() == 11);
	REQUIRE(ink.num_verts() == 15);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 7);
	REQUIRE(float_equal(paths[0].weight, -3));
	REQUIRE(float_equal(paths[1].weight, -1));
	REQUIRE(float_equal(paths[2].weight, 3));
	REQUIRE(float_equal(paths[3].weight, 4));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 8));
	REQUIRE(float_equal(paths[6].weight, 9));
	
	ink.remove_edge("v11", "v13");
	REQUIRE(ink.num_edges() == 10);
	REQUIRE(ink.num_verts() == 15);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 7);
	REQUIRE(float_equal(paths[0].weight, -1));
	REQUIRE(float_equal(paths[1].weight, -1));
	REQUIRE(float_equal(paths[2].weight, 3));
	REQUIRE(float_equal(paths[3].weight, 4));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 8));
	REQUIRE(float_equal(paths[6].weight, 9));
	
	ink.remove_edge("v10", "v12");
	REQUIRE(ink.num_edges() == 9);
	REQUIRE(ink.num_verts() == 15);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 6);
	REQUIRE(float_equal(paths[0].weight, -1));
	REQUIRE(float_equal(paths[1].weight, 3));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 4));
	REQUIRE(float_equal(paths[4].weight, 8));
	REQUIRE(float_equal(paths[5].weight, 9));
}

TEST_CASE("3 Binary Trees" * doctest::timeout(300)) {
	ink::Ink ink;

	build_bt1(ink);
	build_bt2(ink);
	
	
	ink.insert_edge("v16", "v17", 1);
	REQUIRE(ink.num_edges() == 14);
	REQUIRE(ink.num_verts() == 17);
	auto paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 9);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 1));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 8));
	REQUIRE(float_equal(paths[8].weight, 9));

	
	ink.insert_edge("v17", "v18", -3);
	REQUIRE(ink.num_edges() == 15);
	REQUIRE(ink.num_verts() == 18);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 9);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, -2));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 8));
	REQUIRE(float_equal(paths[8].weight, 9));



	ink.insert_edge("v18", "v19", 4);
	REQUIRE(ink.num_edges() == 16);
	REQUIRE(ink.num_verts() == 19);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 9);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 2));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 8));
	REQUIRE(float_equal(paths[8].weight, 9));

	ink.insert_edge("v18", "v20", 4);
	REQUIRE(ink.num_edges() == 17);
	REQUIRE(ink.num_verts() == 20);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 10);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 2));
	REQUIRE(float_equal(paths[4].weight, 2));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 4));
	REQUIRE(float_equal(paths[8].weight, 8));
	REQUIRE(float_equal(paths[9].weight, 9));

	ink.insert_edge("v20", "v21", -11);
	REQUIRE(ink.num_edges() == 18);
	REQUIRE(ink.num_verts() == 21);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 10);
	REQUIRE(float_equal(paths[0].weight, -9));
	REQUIRE(float_equal(paths[1].weight, -5));
	REQUIRE(float_equal(paths[2].weight, -4));
	REQUIRE(float_equal(paths[3].weight, -3));
	REQUIRE(float_equal(paths[4].weight, 2));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 4));
	REQUIRE(float_equal(paths[8].weight, 8));
	REQUIRE(float_equal(paths[9].weight, 9));

	ink.insert_edge("v20", "v22", -12);
	REQUIRE(ink.num_edges() == 19);
	REQUIRE(ink.num_verts() == 22);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 11);
	REQUIRE(float_equal(paths[0].weight, -10));
	REQUIRE(float_equal(paths[1].weight, -9));
	REQUIRE(float_equal(paths[2].weight, -5));
	REQUIRE(float_equal(paths[3].weight, -4));
	REQUIRE(float_equal(paths[4].weight, -3));
	REQUIRE(float_equal(paths[5].weight, 2));
	REQUIRE(float_equal(paths[6].weight, 3));
	REQUIRE(float_equal(paths[7].weight, 4));
	REQUIRE(float_equal(paths[8].weight, 4));
	REQUIRE(float_equal(paths[9].weight, 8));
	REQUIRE(float_equal(paths[10].weight, 9));


	// incremental destruction of binary tree 3
	ink.remove_edge("v20", "v22");
	REQUIRE(ink.num_edges() == 18);
	REQUIRE(ink.num_verts() == 22);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 10);
	REQUIRE(float_equal(paths[0].weight, -9));
	REQUIRE(float_equal(paths[1].weight, -5));
	REQUIRE(float_equal(paths[2].weight, -4));
	REQUIRE(float_equal(paths[3].weight, -3));
	REQUIRE(float_equal(paths[4].weight, 2));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 4));
	REQUIRE(float_equal(paths[8].weight, 8));
	REQUIRE(float_equal(paths[9].weight, 9));

	ink.remove_edge("v20", "v21");
	REQUIRE(ink.num_edges() == 17);
	REQUIRE(ink.num_verts() == 22);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 10);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 2));
	REQUIRE(float_equal(paths[4].weight, 2));
	REQUIRE(float_equal(paths[5].weight, 3));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 4));
	REQUIRE(float_equal(paths[8].weight, 8));
	REQUIRE(float_equal(paths[9].weight, 9));

	ink.remove_edge("v18", "v20");
	REQUIRE(ink.num_edges() == 16);
	REQUIRE(ink.num_verts() == 22);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 9);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 2));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 8));
	REQUIRE(float_equal(paths[8].weight, 9));
	

	ink.remove_edge("v18", "v19");
	REQUIRE(ink.num_edges() == 15);
	REQUIRE(ink.num_verts() == 22);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 9);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, -2));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 8));
	REQUIRE(float_equal(paths[8].weight, 9));

	ink.remove_edge("v17", "v18");
	REQUIRE(ink.num_edges() == 14);
	REQUIRE(ink.num_verts() == 22);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 9);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 1));
	REQUIRE(float_equal(paths[4].weight, 3));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 4));
	REQUIRE(float_equal(paths[7].weight, 8));
	REQUIRE(float_equal(paths[8].weight, 9));

	ink.remove_edge("v16", "v17");
	REQUIRE(ink.num_edges() == 13);
	REQUIRE(ink.num_verts() == 22);
	paths = ink.report_incsfxt(20);
	REQUIRE(paths.size() == 8);
	REQUIRE(float_equal(paths[0].weight, -5));
	REQUIRE(float_equal(paths[1].weight, -4));
	REQUIRE(float_equal(paths[2].weight, -3));
	REQUIRE(float_equal(paths[3].weight, 3));
	REQUIRE(float_equal(paths[4].weight, 4));
	REQUIRE(float_equal(paths[5].weight, 4));
	REQUIRE(float_equal(paths[6].weight, 8));
	REQUIRE(float_equal(paths[7].weight, 9));
}



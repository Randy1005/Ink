#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>
const float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
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




TEST_CASE("1 Binary Tree" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("v1", "v2", 1);
	ink.insert_edge("v1", "v3", 1);
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(ink.num_verts() == 3);
	auto paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 1));
	
	ink.insert_edge("v2", "v4", 2);
	REQUIRE(ink.num_edges() == 3);
	REQUIRE(ink.num_verts() == 4);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	
	ink.insert_edge("v2", "v5", 3);
	REQUIRE(ink.num_edges() == 4);
	REQUIRE(ink.num_verts() == 5);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	REQUIRE(float_equal(paths[2].weight, 4));
	

	ink.insert_edge("v3", "v6", 3);
	REQUIRE(ink.num_edges() == 5);
	REQUIRE(ink.num_verts() == 6);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	
	
	ink.insert_edge("v3", "v7", 3);
	REQUIRE(ink.num_edges() == 6);
	REQUIRE(ink.num_verts() == 7);
	paths = ink.report(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 4));
	
	ink.insert_edge("v6", "v8", 4);
	REQUIRE(ink.num_edges() == 7);
	REQUIRE(ink.num_verts() == 8);
	paths = ink.report(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 8));
	
	
	ink.insert_edge("v6", "v9", 5);
	REQUIRE(ink.num_edges() == 8);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report(10);
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
	paths = ink.report(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 8));
	
	ink.remove_edge("v6", "v8");
	REQUIRE(ink.num_edges() == 6);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report(10);
	REQUIRE(paths.size() == 4);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));
	REQUIRE(float_equal(paths[3].weight, 4));


	ink.remove_edge("v3", "v7");
	REQUIRE(ink.num_edges() == 5);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 3));
	REQUIRE(float_equal(paths[1].weight, 4));
	REQUIRE(float_equal(paths[2].weight, 4));

	ink.remove_edge("v3", "v6");
	REQUIRE(ink.num_edges() == 4);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report(10);
	REQUIRE(paths.size() == 3);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	REQUIRE(float_equal(paths[2].weight, 4));

	ink.remove_edge("v2", "v5");
	REQUIRE(ink.num_edges() == 3);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 3));
	
	ink.remove_edge("v2", "v4");
	REQUIRE(ink.num_edges() == 2);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report(10);
	REQUIRE(paths.size() == 2);
	REQUIRE(float_equal(paths[0].weight, 1));
	REQUIRE(float_equal(paths[1].weight, 1));
	
	ink.remove_edge("v1", "v2");
	ink.remove_edge("v1", "v3");
	REQUIRE(ink.num_edges() == 0);
	REQUIRE(ink.num_verts() == 9);
	paths = ink.report(10);
	REQUIRE(paths.size() == 0);

}

TEST_CASE("2 Binary Trees" * doctest::timeout(300)) {
	ink::Ink ink;

	build_bt1(ink);

	ink.insert_edge("v10", "v11", -1);
	REQUIRE(ink.num_edges() == 9);
	REQUIRE(ink.num_verts() == 11);
	auto paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	auto paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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
	paths = ink.report(20);
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


#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

const	float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}


TEST_CASE("c3_slack Benchmark" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("nx2", "inst_0:A2", 0);
	ink.insert_edge("inst_0:A2", "inst_0:ZN", 10.4);
	ink.insert_edge("nx1", "inst_0:A1", 0);
	ink.insert_edge("nx4", "inst_2:A", 0);
	ink.insert_edge("nx3", "inst_1:A", 0);

	ink.insert_edge("inst_0:A1", "inst_0:ZN", 7.9);
	ink.insert_edge("inst_2:A", "inst_2:Z", 29.4);
	ink.insert_edge("inst_1:A", "inst_1:ZN", 6.72);

	ink.insert_edge("inst_0:ZN", "nx12", 0);
	ink.insert_edge("inst_2:Z", "nx44", 0);
	ink.insert_edge("inst_1:ZN", "nx33", 0);

	REQUIRE(ink.num_edges() == 11);
	REQUIRE(ink.num_verts() == 14);


	auto paths = ink.report(4);
	REQUIRE(paths.size() == 4);

	// shortest path is nx3, inst_1:A, inst_1:ZN, nx33
	// dist = 0, 0, 6.72, 6.72   
	REQUIRE(paths[0].size() == 4);	
	auto it = paths[0].begin();
	REQUIRE(it->vert.name == "nx3");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 6.72f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx33");
	REQUIRE(float_equal(it->dist, 6.72f));

	// 2nd shortest path is nx1, inst_0:A1, inst_0:ZN, nx12
	// dist = 0, 0, 7.9, 7.9   
	REQUIRE(paths[1].size() == 4);	
	it = paths[1].begin();
	REQUIRE(it->vert.name == "nx1");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 7.9f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx12");
	REQUIRE(float_equal(it->dist, 7.9f));

	// 3rd shortest path is nx2, inst_0:A2, inst_0:ZN, nx12
	// dist = 0, 0, 10.4, 10.4   
	REQUIRE(paths[2].size() == 4);	
	it = paths[2].begin();
	REQUIRE(it->vert.name == "nx2");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A2");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 10.4f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx12");
	REQUIRE(float_equal(it->dist, 10.4f));

	// 4th shortest path is nx4, inst_2:A, inst_2:Z, nx44
	// dist = 0, 0, 29.4, 29.4   
	REQUIRE(paths[3].size() == 4);	
	it = paths[3].begin();
	REQUIRE(it->vert.name == "nx4");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:Z");
	REQUIRE(float_equal(it->dist, 29.4f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx44");
	REQUIRE(float_equal(it->dist, 29.4f));

	for (const auto& p : paths) {
		p.dump(std::cout);
	}

}

TEST_CASE("c3_slack Benchmark w/ Incremental Update" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("nx2", "inst_0:A2", 0);
	ink.insert_edge("inst_0:A2", "inst_0:ZN", 10.4);
	ink.insert_edge("nx1", "inst_0:A1", 0);
	ink.insert_edge("nx4", "inst_2:A", 0);
	ink.insert_edge("nx3", "inst_1:A", 0);

	ink.insert_edge("inst_0:A1", "inst_0:ZN", 7.9);
	ink.insert_edge("inst_2:A", "inst_2:Z", 29.4);
	ink.insert_edge("inst_1:A", "inst_1:ZN", 6.72);

	ink.insert_edge("inst_0:ZN", "nx12", 0);
	ink.insert_edge("inst_2:Z", "nx44", 0);
	ink.insert_edge("inst_1:ZN", "nx33", 0);

	REQUIRE(ink.num_edges() == 11);
	REQUIRE(ink.num_verts() == 14);


	auto paths = ink.report(4);
	REQUIRE(paths.size() == 4);

	// shortest path is nx3, inst_1:A, inst_1:ZN, nx33
	// dist = 0, 0, 6.72, 6.72   
	REQUIRE(paths[0].size() == 4);	
	auto it = paths[0].begin();
	REQUIRE(it->vert.name == "nx3");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 6.72f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx33");
	REQUIRE(float_equal(it->dist, 6.72f));

	// 2nd shortest path is nx1, inst_0:A1, inst_0:ZN, nx12
	// dist = 0, 0, 7.9, 7.9   
	REQUIRE(paths[1].size() == 4);	
	it = paths[1].begin();
	REQUIRE(it->vert.name == "nx1");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 7.9f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx12");
	REQUIRE(float_equal(it->dist, 7.9f));

	// 3rd shortest path is nx2, inst_0:A2, inst_0:ZN, nx12
	// dist = 0, 0, 10.4, 10.4   
	REQUIRE(paths[2].size() == 4);	
	it = paths[2].begin();
	REQUIRE(it->vert.name == "nx2");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A2");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 10.4f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx12");
	REQUIRE(float_equal(it->dist, 10.4f));

	// 4th shortest path is nx4, inst_2:A, inst_2:Z, nx44
	// dist = 0, 0, 29.4, 29.4   
	REQUIRE(paths[3].size() == 4);	
	it = paths[3].begin();
	REQUIRE(it->vert.name == "nx4");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:Z");
	REQUIRE(float_equal(it->dist, 29.4f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx44");
	REQUIRE(float_equal(it->dist, 29.4f));

	// -----------------
	// Design Modifiers
	// -----------------
	
	// connect inst_0:A1 to inst_2:Z
	// connect inst_2:A to inst_1:ZN
	ink.insert_edge("inst_0:A1", "inst_2:Z", 9.27);
	ink.insert_edge("inst_2:A", "inst_1:ZN", 9.74);
	REQUIRE(ink.num_edges() == 13);

	// remove inst_1:A
	// connect nx3 to inst_1:ZN
	ink.remove_vertex("inst_1:A");
	ink.insert_edge("nx3", "inst_1:ZN", 6.72);
	REQUIRE(ink.num_edges() == 12);
	REQUIRE(ink.num_verts() == 13);

	// update weight
	// original {10.4, nullopt, ...}
	// new {10.88, nullopt, 13.27, nullopt ...}
	auto& e = ink.insert_edge("inst_0:A2", "inst_0:ZN", 
		10.88, std::nullopt, 13.27);
	REQUIRE(float_equal(*e.weights[0], 10.88f));
	REQUIRE(!e.weights[1]);
	REQUIRE(float_equal(*e.weights[2], 13.27f));
	REQUIRE(!e.weights[3]);
	REQUIRE(!e.weights[4]);
	REQUIRE(!e.weights[5]);
	REQUIRE(!e.weights[6]);
	REQUIRE(!e.weights[7]);
	REQUIRE(ink.num_edges() == 12);

	// add a new vertex and 2 new edges
	ink.insert_edge("nxS", "nx1", 0.33);
	ink.insert_edge("nxS", "nx4", 0.43);
	REQUIRE(ink.num_edges() == 14);
	REQUIRE(ink.num_verts() == 14);

	paths = ink.report(5);
	REQUIRE(paths.size() == 5);
	
	// 1st path
	// ----------------------------------
	// Startpoint:  nx3
	// Endpoint:    nx33
	// Path:
	// Vert name: nx3, Dist: 0
	// Vert name: inst_1:ZN, Dist: 6.72
	// Vert name: nx33, Dist: 6.72
	// ----------------------------------
	REQUIRE(paths[0].size() == 3);
	it = paths[0].begin();
	REQUIRE(it->vert.name == "nx3");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 6.72f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx33");
	REQUIRE(float_equal(it->dist, 6.72f));
	
	// 2nd path
	// ----------------------------------
	// Startpoint:  nxS
	// Endpoint:    nx12
	// Path:
	// Vert name: nxS, Dist: 0
	// Vert name: nx1, Dist: 0.33
	// Vert name: inst_0:A1, Dist: 0.33
	// Vert name: inst_0:ZN, Dist: 8.23
	// Vert name: nx12, Dist: 8.23
	// ----------------------------------
	REQUIRE(paths[1].size() == 5);
	it = paths[1].begin();
	REQUIRE(it->vert.name == "nxS");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx1");
	REQUIRE(float_equal(it->dist, 0.33f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A1");
	REQUIRE(float_equal(it->dist, 0.33f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 8.23f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx12");
	REQUIRE(float_equal(it->dist, 8.23f));
	
	// 3rd path
	// ----------------------------------
	// Startpoint:  nxS
	// Endpoint:    nx44
	// Path:
	// Vert name: nxS, Dist: 0
	// Vert name: nx1, Dist: 0.33
	// Vert name: inst_0:A1, Dist: 0.33
	// Vert name: inst_2:Z, Dist: 9.6
	// Vert name: nx44, Dist: 9.6
	// ----------------------------------
	REQUIRE(paths[2].size() == 5);
	it = paths[2].begin();
	REQUIRE(it->vert.name == "nxS");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx1");
	REQUIRE(float_equal(it->dist, 0.33f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A1");
	REQUIRE(float_equal(it->dist, 0.33f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:Z");
	REQUIRE(float_equal(it->dist, 9.6f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx44");
	REQUIRE(float_equal(it->dist, 9.6f));

	// 4th path
	// ----------------------------------
	// Startpoint:  nxS
	// Endpoint:    nx33
	// Path:
	// Vert name: nxS, Dist: 0
	// Vert name: nx4, Dist: 0.43
	// Vert name: inst_2:A, Dist: 0.43
	// Vert name: inst_1:ZN, Dist: 10.17
	// Vert name: nx33, Dist: 10.17
	// ----------------------------------
	REQUIRE(paths[3].size() == 5);
	it = paths[3].begin();
	REQUIRE(it->vert.name == "nxS");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx4");
	REQUIRE(float_equal(it->dist, 0.43f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:A");
	REQUIRE(float_equal(it->dist, 0.43f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 10.17f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx33");
	REQUIRE(float_equal(it->dist, 10.17f));

	// 5th path
	// ----------------------------------
	// Startpoint:  nx2
	// Endpoint:    nx12
	// Path:
	// Vert name: nx2, Dist: 0
	// Vert name: inst_0:A2, Dist: 0
	// Vert name: inst_0:ZN, Dist: 10.88
	// Vert name: nx12, Dist: 10.88
	// ----------------------------------
	REQUIRE(paths[4].size() == 4);
	it = paths[4].begin();
	REQUIRE(it->vert.name == "nx2");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A2");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 10.88f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx12");
	REQUIRE(float_equal(it->dist, 10.88f));
	
	
	for (const auto& p : paths) {
		p.dump(std::cout);
	}

}



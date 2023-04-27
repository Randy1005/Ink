#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

TEST_CASE("Sample Graph From UITimer Paper" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("v0", "v6",
		2, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v0", "v4",
		4, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v1", "v4",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v2", "v4",
		6, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v2", "v5",
		5, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v3", "v6",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	

	ink.insert_edge("v4", "v6",
		3, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v4", "v7",
		2, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v5", "v7",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v6", "v8",
		6, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v7", "v8",
		3, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	
	auto paths = ink.report(9);

	// 9 paths
	REQUIRE(paths.size() == 9);

	// shortest path is v1, v4, v7, v8
	// dist							 0   1   3   6
	REQUIRE(paths[0].size() == 4);
	auto it = paths[0].begin();
	REQUIRE(it->vert.name == "v1");
	REQUIRE(it->dist == 0.0f);
	// advance to next point in this path
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 1.0f);
	
	// advance to next point
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 3.0f);
	
	
	// advance to next point
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 6.0f);

	// second shortest path is v3, v6, v8
	// dist                     0   1   7
	REQUIRE(paths[1].size() == 3);
	it = paths[1].begin();
	REQUIRE(it->vert.name == "v3");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 1.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 7.0f);

	// third shortest path is v0, v6, v8
	// dist                     0  2   8
	REQUIRE(paths[2].size() == 3);
	it = paths[2].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 2.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 8.0f);

	// fourth shortest path is v0, v4, v7, v8
	// dist                     0   4   6   9
	REQUIRE(paths[3].size() == 4);
	it = paths[3].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 6.0f);


	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 9.0f);

	// fifth shortest path is v2, v5, v7, v8
	// dist                   0   5   6   9
	REQUIRE(paths[4].size() == 4);
	it = paths[4].begin();
	REQUIRE(it->vert.name == "v2");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v5");
	REQUIRE(it->dist == 5.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 6.0f);


	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 9.0f);

	// sixth shortest path is v1, v4, v6, v8
	// dist                   0   1   4   10
	REQUIRE(paths[5].size() == 4);
	it = paths[5].begin();
	REQUIRE(it->vert.name == "v1");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 1.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 10.0f);

	// seventh shortest path is v2, v4, v7, v8
	// dist                      0   6   8  11
	REQUIRE(paths[6].size() == 4);
	it = paths[6].begin();
	REQUIRE(it->vert.name == "v2");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 6.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 8.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 11.0f);

	// eighth shortest path is v0, v4, v6, v8
	// dist                     0   4   7  13
	REQUIRE(paths[7].size() == 4);
	it = paths[7].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 7.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 13.0f);

	// nineth shortest path is v2, v4, v6, v8
	// dist                     0   6   9  15
	REQUIRE(paths[8].size() == 4);
	it = paths[8].begin();
	REQUIRE(it->vert.name == "v2");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 6.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 9.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 15.0f);

}


TEST_CASE("Sample Graph From UITimer Paper w/ Incremental Update" * doctest::timeout(300)) {
	ink::Ink ink;
	ink.insert_edge("v0", "v6",
		2, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v0", "v4",
		4, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v1", "v4",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v2", "v4",
		6, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v2", "v5",
		5, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v3", "v6",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	

	ink.insert_edge("v4", "v6",
		3, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v4", "v7",
		2, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v5", "v7",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v6", "v8",
		6, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v7", "v8",
		3, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	
	auto paths = ink.report(9);

	// 9 paths
	REQUIRE(paths.size() == 9);

	// shortest path is v1, v4, v7, v8
	// dist							 0   1   3   6
	REQUIRE(paths[0].size() == 4);
	auto it = paths[0].begin();
	REQUIRE(it->vert.name == "v1");
	REQUIRE(it->dist == 0.0f);
	// advance to next point in this path
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 1.0f);
	
	// advance to next point
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 3.0f);
	
	
	// advance to next point
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 6.0f);

	// second shortest path is v3, v6, v8
	// dist                     0   1   7
	REQUIRE(paths[1].size() == 3);
	it = paths[1].begin();
	REQUIRE(it->vert.name == "v3");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 1.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 7.0f);

	// third shortest path is v0, v6, v8
	// dist                     0  2   8
	REQUIRE(paths[2].size() == 3);
	it = paths[2].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 2.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 8.0f);

	// fourth shortest path is v0, v4, v7, v8
	// dist                     0   4   6   9
	REQUIRE(paths[3].size() == 4);
	it = paths[3].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 6.0f);


	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 9.0f);

	// fifth shortest path is v2, v5, v7, v8
	// dist                   0   5   6   9
	REQUIRE(paths[4].size() == 4);
	it = paths[4].begin();
	REQUIRE(it->vert.name == "v2");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v5");
	REQUIRE(it->dist == 5.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 6.0f);


	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 9.0f);

	// sixth shortest path is v1, v4, v6, v8
	// dist                   0   1   4   10
	REQUIRE(paths[5].size() == 4);
	it = paths[5].begin();
	REQUIRE(it->vert.name == "v1");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 1.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 10.0f);

	// seventh shortest path is v2, v4, v7, v8
	// dist                      0   6   8  11
	REQUIRE(paths[6].size() == 4);
	it = paths[6].begin();
	REQUIRE(it->vert.name == "v2");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 6.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 8.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 11.0f);

	// eighth shortest path is v0, v4, v6, v8
	// dist                     0   4   7  13
	REQUIRE(paths[7].size() == 4);
	it = paths[7].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 7.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 13.0f);

	// nineth shortest path is v2, v4, v6, v8
	// dist                     0   6   9  15
	REQUIRE(paths[8].size() == 4);
	it = paths[8].begin();
	REQUIRE(it->vert.name == "v2");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 6.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 9.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 15.0f);

	// ---------------------------
	// design modifier operations
	// ---------------------------
	
	// removing vertices v1, v2 
	ink.remove_vertex("v1");
	ink.remove_vertex("v2");
	

	paths = ink.report(5);
	REQUIRE(paths.size() == 5);
	
	
	// 1st path
	// ---------------------
	// Startpoint:  v5
	// Endpoint:    v8
	// Path:
	// Vert name: v5, Dist: 0
	// Vert name: v7, Dist: 1
	// Vert name: v8, Dist: 4
	// ---------------------
	REQUIRE(paths[0].size() == 3);
	it = paths[0].begin();
	REQUIRE(it->vert.name == "v5");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 1.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 4.0f);

	// 2nd path
	//----------------------------------
	// Startpoint:  v3
	// Endpoint:    v8
	// Path:
	// Vert name: v3, Dist: 0
	// Vert name: v6, Dist: 1
	// Vert name: v8, Dist: 7
	//----------------------------------
	REQUIRE(paths[1].size() == 3);
	it = paths[1].begin();
	REQUIRE(it->vert.name == "v3");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 1.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 7.0f);

	// 3rd path
	//----------------------------------
	// Startpoint:  v0
	// Endpoint:    v8
	// Path:
	// Vert name: v0, Dist: 0
	// Vert name: v6, Dist: 2
	// Vert name: v8, Dist: 8
	//----------------------------------
	REQUIRE(paths[2].size() == 3);
	it = paths[2].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 2.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 8.0f);

	// 4th path 
	//----------------------------------
	// Startpoint:  v0
	// Endpoint:    v8
	// Path:
	// Vert name: v0, Dist: 0
	// Vert name: v4, Dist: 4
	// Vert name: v7, Dist: 6
	// Vert name: v8, Dist: 9
	//----------------------------------
	REQUIRE(paths[3].size() == 4);
	it = paths[3].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v7");
	REQUIRE(it->dist == 6.0f);
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 9.0f);

	//----------------------------------
	// Startpoint:  v0
	// Endpoint:    v8
	// Path:
	// Vert name: v0, Dist: 0
	// Vert name: v4, Dist: 4
	// Vert name: v6, Dist: 7
	// Vert name: v8, Dist: 13
	//----------------------------------
	REQUIRE(paths[4].size() == 4);
	it = paths[4].begin();
	REQUIRE(it->vert.name == "v0");
	REQUIRE(it->dist == 0.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v4");
	REQUIRE(it->dist == 4.0f);

	std::advance(it, 1);
	REQUIRE(it->vert.name == "v6");
	REQUIRE(it->dist == 7.0f);
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "v8");
	REQUIRE(it->dist == 13.0f);



	for (const auto& p : paths) {
		p.dump(std::cout);
	}
}



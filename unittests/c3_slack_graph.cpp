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

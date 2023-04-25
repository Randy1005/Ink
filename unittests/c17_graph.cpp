#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

const	float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}


TEST_CASE("c17 Benchmark" * doctest::timeout(300)) {
	ink::Ink ink;


	ink.insert_edge("nx1", "inst_1:A1", 0);
	ink.insert_edge("nx3", "inst_1:A2", 0);
	ink.insert_edge("nx3", "inst_0:A1", 0);
	ink.insert_edge("nx6", "inst_0:A2", 0);
	ink.insert_edge("nx7", "inst_2:A1", 0);
	ink.insert_edge("nx2", "inst_3:A1", 0);

	ink.insert_edge("inst_0:A1", "inst_0:ZN", 7.54);
	ink.insert_edge("inst_0:A2", "inst_0:ZN", 10);

	ink.insert_edge("inst_1:A1", "inst_1:ZN", 6.59);
	ink.insert_edge("inst_1:A2", "inst_1:ZN", 9.1);

	ink.insert_edge("inst_3:A1", "inst_3:ZN", 7.5);
	ink.insert_edge("inst_3:A2", "inst_3:ZN", 9.99, 9.98);
	
	ink.insert_edge("inst_0:ZN", "inst_3:A2", 0);
	ink.insert_edge("inst_0:ZN", "inst_2:A2", 0);
	
	ink.insert_edge("inst_2:A2", "inst_2:ZN", 9.14, 9.13);
	ink.insert_edge("inst_2:A1", "inst_2:ZN", 6.63);

	ink.insert_edge("inst_1:ZN", "inst_5:A1", 0);
	ink.insert_edge("inst_3:ZN", "inst_5:A2", 0);
	ink.insert_edge("inst_3:ZN", "inst_4:A1", 0);

	ink.insert_edge("inst_2:ZN", "inst_4:A2", 0);

	ink.insert_edge("inst_5:A1", "inst_5:ZN", 7.9, 7.88);
	ink.insert_edge("inst_5:A2", "inst_5:ZN", 10.4);

	ink.insert_edge("inst_5:ZN", "nx22", 0);

	ink.insert_edge("inst_4:A1", "inst_4:ZN", 7.91, 7.89);
	ink.insert_edge("inst_4:A2", "inst_4:ZN", 10.4);

	ink.insert_edge("inst_4:ZN", "nx23", 0);

	REQUIRE(ink.num_edges() == 26);
	REQUIRE(ink.num_verts() == 25);
	auto paths = ink.report(7);
	
	REQUIRE(paths.size() == 7);

	// shortest path is nx1, inst_1:A1, inst_1:ZN, inst_5:A1, inst_5:ZN, nx22
	// dist =						  0, 0, 6.59, 6.59, 14.47, 14.47   
	REQUIRE(paths[0].size() == 6);	
	auto it = paths[0].begin();
	REQUIRE(it->vert.name == "nx1");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 6.59f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A1");
	REQUIRE(float_equal(it->dist, 6.59f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 14.47f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx22");
	REQUIRE(float_equal(it->dist, 14.47f));

	// second shortest path is nx1, inst_1:A1, inst_1:ZN, inst_5:A1, inst_5:ZN, nx22
	// dist =										0, 0, 6.59, 6.59, 14.49, 14.49   
	REQUIRE(paths[1].size() == 6);	
	it = paths[1].begin();
	REQUIRE(it->vert.name == "nx1");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 6.59f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A1");
	REQUIRE(float_equal(it->dist, 6.59f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 14.49f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx22");
	REQUIRE(float_equal(it->dist, 14.49f));

	// third shortest path is nx2, inst_3:A1, inst_3:ZN, inst_4:A1, inst_4:ZN, nx23
	// dist =						  0, 0, 7.5, 7.5, 15.39, 15.39   
	REQUIRE(paths[2].size() == 6);	
	it = paths[2].begin();
	REQUIRE(it->vert.name == "nx2");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_3:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_3:ZN");
	REQUIRE(float_equal(it->dist, 7.5f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:A1");
	REQUIRE(float_equal(it->dist, 7.5f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:ZN");
	REQUIRE(float_equal(it->dist, 15.39f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx23");
	REQUIRE(float_equal(it->dist, 15.39f));

	// fourth shortest path is nx2, inst_3:A1, inst_3:ZN, inst_4:A1, inst_4:ZN, nx23
	// dist =						  0, 0, 7.5, 7.5, 15.41, 15.41   
	REQUIRE(paths[3].size() == 6);	
	it = paths[3].begin();
	REQUIRE(it->vert.name == "nx2");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_3:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_3:ZN");
	REQUIRE(float_equal(it->dist, 7.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:A1");
	REQUIRE(float_equal(it->dist, 7.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:ZN");
	REQUIRE(float_equal(it->dist, 15.41f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx23");
	REQUIRE(float_equal(it->dist, 15.41f));

	// fifth shortest path is nx3, inst_1:A2, inst_1:ZN, inst_5:A1, inst_5:ZN, nx22
	// dist =						  0, 0, 9.1, 9.1, 16.98, 16.98   
	REQUIRE(paths[4].size() == 6);	
	it = paths[4].begin();
	REQUIRE(it->vert.name == "nx3");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A2");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 9.1f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A1");
	REQUIRE(float_equal(it->dist, 9.1f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 16.98f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx22");
	REQUIRE(float_equal(it->dist, 16.98f));

	// sixth shortest path is nx3, inst_1:A2, inst_1:ZN, inst_5:A1, inst_5:ZN, nx22
	// dist =						  0, 0, 9.1, 9.1, 17, 17   
	REQUIRE(paths[5].size() == 6);	
	it = paths[5].begin();
	REQUIRE(it->vert.name == "nx3");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A2");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 9.1f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A1");
	REQUIRE(float_equal(it->dist, 9.1f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 17.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx22");
	REQUIRE(float_equal(it->dist, 17.0f));

	// seventh shortest path is nx7, inst_2:A1, inst_2:ZN, inst_4:A2, inst_4:ZN, nx23
	// dist =						  0, 0, 6.63, 6.63, 17.03, 17.03   
	REQUIRE(paths[6].size() == 6);	
	it = paths[6].begin();
	REQUIRE(it->vert.name == "nx7");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:ZN");
	REQUIRE(float_equal(it->dist, 6.63f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:A2");
	REQUIRE(float_equal(it->dist, 6.63f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:ZN");
	REQUIRE(float_equal(it->dist, 17.03f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "nx23");
	REQUIRE(float_equal(it->dist, 17.03f));

	for (const auto& p : paths) {
		p.dump(std::cout);
	}

}

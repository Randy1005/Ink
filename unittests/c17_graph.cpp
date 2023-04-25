#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

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





	ink.dump(std::cout);

	auto paths = ink.report(10);
	
	//for (const auto& p : paths) {
	//	p.dump(std::cout);
	//}

}

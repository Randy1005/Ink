#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

const	float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}


TEST_CASE("Simplified s27 Benchmark" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("reset_net", "inst_13:A", 0);
	ink.insert_edge("inst_13:A", "inst_13:ZN", 7.86);
	ink.insert_edge("inst_13:ZN", "inst_14:RN", 0);
	ink.insert_edge("inst_13:ZN", "inst_16:RN", 0);
	ink.insert_edge("inst_13:ZN", "inst_15:RN", 0);

	ink.insert_edge("clk_net", "inst_18:A", 0);
	ink.insert_edge("inst_18:A", "inst_18:Z", 31.5);

	ink.insert_edge("inst_18:Z", "inst_26:A", 0);
	ink.insert_edge("inst_26:A", "inst_26:Z", 31.1);
	ink.insert_edge("inst_26:Z", "inst_27:A", 0);
	ink.insert_edge("inst_27:A", "inst_27:Z", 31.1);
	ink.insert_edge("inst_27:Z", "inst_28:A", 0);
	ink.insert_edge("inst_28:A", "inst_28:Z", 31);
	ink.insert_edge("inst_28:Z", "inst_14:CK", 0);
	ink.insert_edge("inst_14:CK", "inst_14:Q", 127);
	ink.insert_edge("inst_14:CK", "inst_14:QN", 87.4);
	ink.insert_edge("inst_14:CK", "inst_14:D", 0);
	
	
	ink.insert_edge("inst_14:QN", "inst_7:A2", 0);
	ink.insert_edge("inst_7:A2", "inst_7:ZN", 8.99);
	ink.insert_edge("inst_7:ZN", "inst_9:A", 0);
	ink.insert_edge("inst_9:A", "inst_9:ZN", 4.94, 4.93);
	ink.insert_edge("inst_9:ZN", "inst_5:A2", 0);
	ink.insert_edge("inst_5:A2", "inst_5:ZN", 8.89);
	ink.insert_edge("inst_5:ZN", "inst_14:D", 0);

	// G2
	ink.insert_edge("G2", "inst_5:A1", 0);
	ink.insert_edge("inst_5:A1", "inst_5:ZN", 6.77);

	// G1
	ink.insert_edge("G1", "inst_10:A", 0);
	ink.insert_edge("inst_10:A", "inst_10:ZN", 4.98);
	ink.insert_edge("inst_10:ZN", "inst_7:A1", 0);
	ink.insert_edge("inst_7:A1", "inst_7:ZN", 6.31);

	REQUIRE(ink.num_edges() == 30);
	REQUIRE(ink.num_verts() == 31);


	auto paths = ink.report(10);
	
	REQUIRE(paths.size() == 10);

	// shortest path is G2, isnt_5:A1, inst_5:ZN, inst_14:D
	// dist =	0, 0, 6.77, 6.77 
	REQUIRE(paths[0].size() == 4);	
	auto it = paths[0].begin();
	REQUIRE(it->vert.name == "G2");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A1");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 6.77f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 6.77f));

	// 2nd shortest path is reset_net, inst_13:A, inst_13:ZN, inst_15:RN
	// dist = 0, 0, 7.86, 7.86
	REQUIRE(paths[1].size() == 4);	
	it = paths[1].begin();
	REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 7.86f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_15:RN");
	REQUIRE(float_equal(it->dist, 7.86f));

	// 3rd shortest path is reset_net, inst_13:A, inst_13:ZN, inst_16:RN
	// dist = 0, 0, 7.86, 7.86
	REQUIRE(paths[2].size() == 4);	
	it = paths[2].begin();
	REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 7.86f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_16:RN");
	REQUIRE(float_equal(it->dist, 7.86f));
	
	// 4th shortest path is reset_net, inst_13:A, inst_13:ZN, inst_14:RN
	// dist = 0, 0, 7.86, 7.86
	REQUIRE(paths[3].size() == 4);	
	it = paths[3].begin();
	REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 7.86f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:RN");
	REQUIRE(float_equal(it->dist, 7.86f));

	// 5th shortest path is:
	// G1, inst_10:A, inst_10:ZN, inst_7:A1, inst_7:ZN, inst_9:A, inst_9:ZN,
	// inst_5:A2, inst_5:ZN, inst_14:D
	// dist = 0, 0, 4.98, 4.98, 11.29, 11.29, 16.22, 16.22, 25.11, 25.11
	REQUIRE(paths[4].size() == 10);	
	it = paths[4].begin();
	REQUIRE(it->vert.name == "G1");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:ZN");
	REQUIRE(float_equal(it->dist, 4.98f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:A1");
	REQUIRE(float_equal(it->dist, 4.98f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:ZN");
	REQUIRE(float_equal(it->dist, 11.29f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:A");
	REQUIRE(float_equal(it->dist, 11.29f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:ZN");
	REQUIRE(float_equal(it->dist, 16.22f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A2");
	REQUIRE(float_equal(it->dist, 16.22f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 25.11f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 25.11f));

	// 6th shortest path is:
	// G1, inst_10:A, inst_10:ZN, inst_7:A1, inst_7:ZN, inst_9:A, inst_9:ZN,
	// inst_5:A2, inst_5:ZN, inst_14:D
	// dist = 0, 0, 4.98, 4.98, 11.29, 11.29, 16.23, 16.23, 25.12, 25.12
	REQUIRE(paths[5].size() == 10);	
	it = paths[5].begin();
	REQUIRE(it->vert.name == "G1");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:ZN");
	REQUIRE(float_equal(it->dist, 4.98f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:A1");
	REQUIRE(float_equal(it->dist, 4.98f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:ZN");
	REQUIRE(float_equal(it->dist, 11.29f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:A");
	REQUIRE(float_equal(it->dist, 11.29f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:ZN");
	REQUIRE(float_equal(it->dist, 16.23f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A2");
	REQUIRE(float_equal(it->dist, 16.23f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 25.12f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 25.12f));

	// 7th shortest path is:
	// clk_net(0), inst_18:A(0), inst_18:Z(31.5), inst_26:A(31.5), inst_26:Z(62.6), 
	// inst_27:A(62.6), inst_27:Z(93.7), inst_28:A(93.7), inst_28:Z(124.7),
	// inst_14:CK(124.7), inst_14:D(124.7)
	
	REQUIRE(paths[6].size() == 11);	
	it = paths[6].begin();
	REQUIRE(it->vert.name == "clk_net");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:Z");
	REQUIRE(float_equal(it->dist, 31.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:A");
	REQUIRE(float_equal(it->dist, 31.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:Z");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:A");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:Z");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:A");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:Z");
	REQUIRE(float_equal(it->dist, 124.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:CK");
	REQUIRE(float_equal(it->dist, 124.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 124.7f));
	
	// 8th shortest path	
	// --------------------------------
	//	Startpoint:  clk_net
	//	Endpoint:    inst_14:D
	//	Path:
	//	Vert name: clk_net, Dist: 0
	//	Vert name: inst_18:A, Dist: 0
	//	Vert name: inst_18:Z, Dist: 31.5
	//	Vert name: inst_26:A, Dist: 31.5
	//	Vert name: inst_26:Z, Dist: 62.6
	//	Vert name: inst_27:A, Dist: 62.6
	//	Vert name: inst_27:Z, Dist: 93.7
	//	Vert name: inst_28:A, Dist: 93.7
	//	Vert name: inst_28:Z, Dist: 124.7
	//	Vert name: inst_14:CK, Dist: 124.7
	//	Vert name: inst_14:QN, Dist: 212.1
	//	Vert name: inst_7:A2, Dist: 212.1
	//	Vert name: inst_7:ZN, Dist: 221.09
	//	Vert name: inst_9:A, Dist: 221.09
	//	Vert name: inst_9:ZN, Dist: 226.02
	//	Vert name: inst_5:A2, Dist: 226.02
	//	Vert name: inst_5:ZN, Dist: 234.91
	//	Vert name: inst_14:D, Dist: 234.91
	// --------------------------------
	REQUIRE(paths[7].size() == 18);	
	it = paths[7].begin();
	REQUIRE(it->vert.name == "clk_net");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:Z");
	REQUIRE(float_equal(it->dist, 31.5f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:A");
	REQUIRE(float_equal(it->dist, 31.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:Z");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:A");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:Z");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:A");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:Z");
	REQUIRE(float_equal(it->dist, 124.7f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:CK");
	REQUIRE(float_equal(it->dist, 124.7f));
	
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:QN");
	REQUIRE(float_equal(it->dist, 212.1f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:A2");
	REQUIRE(float_equal(it->dist, 212.1f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:ZN");
	REQUIRE(float_equal(it->dist, 221.09f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:A");
	REQUIRE(float_equal(it->dist, 221.09f));
		
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:ZN");
	REQUIRE(float_equal(it->dist, 226.02f));


	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A2");
	REQUIRE(float_equal(it->dist, 226.02f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 234.91f));
	
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 234.91f));
	
	// 9th shortest path	
	// --------------------------------
	//	Startpoint:  clk_net
	//	Endpoint:    inst_14:D
	//	Path:
	//	Vert name: clk_net, Dist: 0
	//	Vert name: inst_18:A, Dist: 0
	//	Vert name: inst_18:Z, Dist: 31.5
	//	Vert name: inst_26:A, Dist: 31.5
	//	Vert name: inst_26:Z, Dist: 62.6
	//	Vert name: inst_27:A, Dist: 62.6
	//	Vert name: inst_27:Z, Dist: 93.7
	//	Vert name: inst_28:A, Dist: 93.7
	//	Vert name: inst_28:Z, Dist: 124.7
	//	Vert name: inst_14:CK, Dist: 124.7
	//	Vert name: inst_14:QN, Dist: 212.1
	//	Vert name: inst_7:A2, Dist: 212.1
	//	Vert name: inst_7:ZN, Dist: 221.09
	//	Vert name: inst_9:A, Dist: 221.09
	//	Vert name: inst_9:ZN, Dist: 226.03
	//	Vert name: inst_5:A2, Dist: 226.03
	//	Vert name: inst_5:ZN, Dist: 234.92
	//	Vert name: inst_14:D, Dist: 234.92
	// --------------------------------
	REQUIRE(paths[8].size() == 18);	
	it = paths[8].begin();
	REQUIRE(it->vert.name == "clk_net");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:Z");
	REQUIRE(float_equal(it->dist, 31.5f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:A");
	REQUIRE(float_equal(it->dist, 31.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:Z");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:A");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:Z");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:A");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:Z");
	REQUIRE(float_equal(it->dist, 124.7f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:CK");
	REQUIRE(float_equal(it->dist, 124.7f));
	
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:QN");
	REQUIRE(float_equal(it->dist, 212.1f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:A2");
	REQUIRE(float_equal(it->dist, 212.1f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:ZN");
	REQUIRE(float_equal(it->dist, 221.09f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:A");
	REQUIRE(float_equal(it->dist, 221.09f));
		
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:ZN");
	REQUIRE(float_equal(it->dist, 226.03f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A2");
	REQUIRE(float_equal(it->dist, 226.03f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 234.92f));
	
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 234.92f));

	// 10th shortest path
	// -----------------------------
	//	Startpoint:  clk_net
	//	Endpoint:    inst_14:Q
	//	Path:
	//	Vert name: clk_net, Dist: 0
	//	Vert name: inst_18:A, Dist: 0
	//	Vert name: inst_18:Z, Dist: 31.5
	//	Vert name: inst_26:A, Dist: 31.5
	//	Vert name: inst_26:Z, Dist: 62.6
	//	Vert name: inst_27:A, Dist: 62.6
	//	Vert name: inst_27:Z, Dist: 93.7
	//	Vert name: inst_28:A, Dist: 93.7
	//	Vert name: inst_28:Z, Dist: 124.7
	//	Vert name: inst_14:CK, Dist: 124.7
	//	Vert name: inst_14:Q, Dist: 251.7
	// -----------------------------
	REQUIRE(paths[9].size() == 11);	
	it = paths[9].begin();
	REQUIRE(it->vert.name == "clk_net");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:Z");
	REQUIRE(float_equal(it->dist, 31.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:A");
	REQUIRE(float_equal(it->dist, 31.5f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:Z");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:A");
	REQUIRE(float_equal(it->dist, 62.6f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:Z");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:A");
	REQUIRE(float_equal(it->dist, 93.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:Z");
	REQUIRE(float_equal(it->dist, 124.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:CK");
	REQUIRE(float_equal(it->dist, 124.7f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:Q");
	REQUIRE(float_equal(it->dist, 251.7f));
	
	for (const auto& p : paths) {
		p.dump(std::cout);
	}

}

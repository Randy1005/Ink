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


	auto paths = ink.report_incsfxt(10);
	
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


	// NOTE: the order of the 2nd, 3rd, 4th path are interchangeable
	

	// 2nd shortest path is reset_net, inst_13:A, inst_13:ZN, inst_15:RN
	// dist = 0, 0, 7.86, 7.86
	REQUIRE(paths[1].size() == 4);	
	it = paths[1].begin();
	REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0.0f));
	//
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 7.86f));


	std::advance(it, 1);
	//REQUIRE(it->vert.name == "inst_15:RN");
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
	//REQUIRE(it->vert.name == "inst_16:RN");
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
	//REQUIRE(it->vert.name == "inst_14:RN");
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


TEST_CASE("Simplified s27 Benchmark w/ Incremental Update" * doctest::timeout(300)) {
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


	auto paths = ink.report_incsfxt(10);
	
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

	// NOTE: The order of the 2nd, 3rd, 4th path are interchangeable
	// they have the same weight 7.86 (order is indeterministic because of 
	// taskflow.transform_reduce)
	
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
	//REQUIRE(it->vert.name == "inst_15:RN");
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
	//REQUIRE(it->vert.name == "inst_16:RN");
	REQUIRE(float_equal(it->dist, 7.86f));
	
	// 4th shortest path is reset_net, inst_13:A, inst_13:ZN, inst_14:RN
	// dist = 0, 0, 7.86, 7.86
	REQUIRE(paths[3].size() == 4);	
	it = paths[3].begin();
	//REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0.0f));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0.0f));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 7.86f));


	std::advance(it, 1);
	//REQUIRE(it->vert.name == "inst_14:RN");
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



	// -----------------------
	// Design Modifiers
	// -----------------------
	
	// remove vertex: 4 edges should be removed too
	ink.remove_vertex("inst_14:CK");

	REQUIRE(ink.num_edges() == 26);
	REQUIRE(ink.num_verts() == 30);
	
	// connect inst_28:Z to inst_14:QN, inst_14:Q, and inst_14:D
	ink.insert_edge("inst_28:Z", "inst_14:QN", 87.4);
	ink.insert_edge("inst_28:Z", "inst_14:Q", 127);
	ink.insert_edge("inst_28:Z", "inst_14:D", 0);

	REQUIRE(ink.num_edges() == 29);
	
	// add more edges
	ink.insert_edge("inst_18:Z", "inst_19:A", 0);
	ink.insert_edge("inst_19:A", "inst_19:Z", 31.1);
	ink.insert_edge("inst_19:Z", "inst_20:A", 0);
	ink.insert_edge("inst_20:A", "inst_20:Z", 31.4);
	ink.insert_edge("inst_20:Z", "inst_15:CK", 0);
	ink.insert_edge("inst_15:CK", "inst_15:Q", 127);
	ink.insert_edge("inst_15:CK", "inst_15:QN", 88.2);

	REQUIRE(ink.num_edges() == 36);
	REQUIRE(ink.num_verts() == 37);

	// modify edge weights
	const auto& e0 = ink.insert_edge("inst_13:A", "inst_13:ZN", 187, 24.3);
	REQUIRE(float_equal(*e0.weights[0], 187));
	REQUIRE(float_equal(*e0.weights[1], 24.3));
	REQUIRE(!e0.weights[2]);
	REQUIRE(!e0.weights[3]);
	REQUIRE(!e0.weights[4]);
	REQUIRE(!e0.weights[5]);
	REQUIRE(!e0.weights[6]);
	REQUIRE(!e0.weights[7]);

	const auto& e1 = ink.insert_edge("inst_7:A1", "inst_7:ZN", 32.5);
	REQUIRE(float_equal(*e1.weights[0], 32.5));
	REQUIRE(!e1.weights[1]);
	REQUIRE(!e1.weights[2]);
	REQUIRE(!e1.weights[3]);
	REQUIRE(!e1.weights[4]);
	REQUIRE(!e1.weights[5]);
	REQUIRE(!e1.weights[6]);
	REQUIRE(!e1.weights[7]);
	
	const auto& e2 = ink.insert_edge("G2", "inst_5:A1", 9.83, 0);
	REQUIRE(float_equal(*e2.weights[0], 9.83));
	REQUIRE(float_equal(*e2.weights[1], 0));
	REQUIRE(!e2.weights[2]);
	REQUIRE(!e2.weights[3]);
	REQUIRE(!e2.weights[4]);
	REQUIRE(!e2.weights[5]);
	REQUIRE(!e2.weights[6]);
	REQUIRE(!e2.weights[7]);

	// report paths again
	paths = ink.report_incsfxt(8);
	REQUIRE(paths.size() == 8);

	// 1st path
	// ----------------------------------
	// Startpoint:  G2
	// Endpoint:    inst_14:D
	// Path:
	// Vert name: G2, Dist: 0
	// Vert name: inst_5:A1, Dist: 0
	// Vert name: inst_5:ZN, Dist: 6.77
	// Vert name: inst_14:D, Dist: 6.77
	// ----------------------------------	
	REQUIRE(paths[0].size() == 4);
	it = paths[0].begin();
	REQUIRE(it->vert.name == "G2");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A1");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 6.77));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 6.77));
	
	// 2nd path
	// ----------------------------------
	// Startpoint:  G2
	// Endpoint:    inst_14:D
	// Path:
	// Vert name: G2, Dist: 0
	// Vert name: inst_5:A1, Dist: 9.83
	// Vert name: inst_5:ZN, Dist: 16.6
	// Vert name: inst_14:D, Dist: 16.6
	// ----------------------------------
	REQUIRE(paths[1].size() == 4);
	it = paths[1].begin();
	REQUIRE(it->vert.name == "G2");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A1");
	REQUIRE(float_equal(it->dist, 9.83));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 16.6));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 16.6));
	


	// NOTE: the order of the 3rd, 4th, 5th path are interchangeable
	// they have the same path weight 24.3
	
	REQUIRE(paths[2].size() == 4);
	it = paths[2].begin();
	REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 24.3));

	std::advance(it, 1);
	// NOTE: the final vertex could be inst_[14, 16, 15]:RN
	REQUIRE(float_equal(it->dist, 24.3));

	REQUIRE(paths[3].size() == 4);
	it = paths[3].begin();
	REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 24.3));

	std::advance(it, 1);
	// NOTE: the final vertex could be inst_[14, 16, 15]:RN
	REQUIRE(float_equal(it->dist, 24.3));
	
	REQUIRE(paths[4].size() == 4);
	it = paths[4].begin();
	REQUIRE(it->vert.name == "reset_net");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:A");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_13:ZN");
	REQUIRE(float_equal(it->dist, 24.3));

	std::advance(it, 1);
	// NOTE: the final vertex could be inst_[14, 16, 15]:RN
	REQUIRE(float_equal(it->dist, 24.3));
	

	// 6th path
	// ----------------------------------
	// Startpoint:  G1
	// Endpoint:    inst_14:D
	// Path:
	// Vert name: G1, Dist: 0
	// Vert name: inst_10:A, Dist: 0
	// Vert name: inst_10:ZN, Dist: 4.98
	// Vert name: inst_7:A1, Dist: 4.98
	// Vert name: inst_7:ZN, Dist: 37.48
	// Vert name: inst_9:A, Dist: 37.48
	// Vert name: inst_9:ZN, Dist: 42.41
	// Vert name: inst_5:A2, Dist: 42.41
	// Vert name: inst_5:ZN, Dist: 51.3
	// Vert name: inst_14:D, Dist: 51.3
	// ----------------------------------
	REQUIRE(paths[5].size() == 10);
	it = paths[5].begin();
	REQUIRE(it->vert.name == "G1");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:A");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:ZN");
	REQUIRE(float_equal(it->dist, 4.98));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:A1");
	REQUIRE(float_equal(it->dist, 4.98));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:ZN");
	REQUIRE(float_equal(it->dist, 37.48));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:A");
	REQUIRE(float_equal(it->dist, 37.48));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:ZN");
	REQUIRE(float_equal(it->dist, 42.41));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A2");
	REQUIRE(float_equal(it->dist, 42.41));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 51.3));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 51.3));

	// 7th path
	// ----------------------------------
	// Startpoint:  G1
	// Endpoint:    inst_14:D
	// Path:
	// Vert name: G1, Dist: 0
	// Vert name: inst_10:A, Dist: 0
	// Vert name: inst_10:ZN, Dist: 4.98
	// Vert name: inst_7:A1, Dist: 4.98
	// Vert name: inst_7:ZN, Dist: 37.48
	// Vert name: inst_9:A, Dist: 37.48
	// Vert name: inst_9:ZN, Dist: 42.42
	// Vert name: inst_5:A2, Dist: 42.42
	// Vert name: inst_5:ZN, Dist: 51.31
	// Vert name: inst_14:D, Dist: 51.31
	// ----------------------------------
	REQUIRE(paths[6].size() == 10);
	it = paths[6].begin();
	REQUIRE(it->vert.name == "G1");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:A");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_10:ZN");
	REQUIRE(float_equal(it->dist, 4.98));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:A1");
	REQUIRE(float_equal(it->dist, 4.98));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_7:ZN");
	REQUIRE(float_equal(it->dist, 37.48));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:A");
	REQUIRE(float_equal(it->dist, 37.48));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_9:ZN");
	REQUIRE(float_equal(it->dist, 42.42));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:A2");
	REQUIRE(float_equal(it->dist, 42.42));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_5:ZN");
	REQUIRE(float_equal(it->dist, 51.31));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 51.31));

	// 8th path
	// ----------------------------------
	// Startpoint:  clk_net
	// Endpoint:    inst_14:D
	// Path:
	// Vert name: clk_net, Dist: 0
	// Vert name: inst_18:A, Dist: 0
	// Vert name: inst_18:Z, Dist: 31.5
	// Vert name: inst_26:A, Dist: 31.5
	// Vert name: inst_26:Z, Dist: 62.6
	// Vert name: inst_27:A, Dist: 62.6
	// Vert name: inst_27:Z, Dist: 93.7
	// Vert name: inst_28:A, Dist: 93.7
	// Vert name: inst_28:Z, Dist: 124.7
	// Vert name: inst_14:D, Dist: 124.7
	// ----------------------------------
	REQUIRE(paths[7].size() == 10);
	it = paths[7].begin();
	REQUIRE(it->vert.name == "clk_net");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:A");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_18:Z");
	REQUIRE(float_equal(it->dist, 31.5));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:A");
	REQUIRE(float_equal(it->dist, 31.5));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_26:Z");
	REQUIRE(float_equal(it->dist, 62.6));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:A");
	REQUIRE(float_equal(it->dist, 62.6));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_27:Z");
	REQUIRE(float_equal(it->dist, 93.7));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:A");
	REQUIRE(float_equal(it->dist, 93.7));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_28:Z");
	REQUIRE(float_equal(it->dist, 124.7));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_14:D");
	REQUIRE(float_equal(it->dist, 124.7));


	for (const auto& p : paths) {
		p.dump(std::cout);
	}

}

TEST_CASE("Simplified s27 Benchmark w/ Incremental Update(2)" * doctest::timeout(300)) {
	ink::Ink ink;
	
	ink.insert_edge("inst_15:CK", "inst_15:Q", 127);
	ink.insert_edge("inst_15:CK", "inst_15:QN", 88.2);
	
	ink.insert_edge("inst_15:QN", "inst_4:A1", 0);
	ink.insert_edge("inst_15:QN", "inst_3:A1", 0);

	ink.insert_edge("G0", "inst_11:A", 0);
	ink.insert_edge("G0", "inst_3:A2", 0);
	ink.insert_edge("G0", "inst_4:A2", 0);

	ink.insert_edge("inst_3:A2", "inst_3:ZN", 10.3);
	ink.insert_edge("inst_3:A1", "inst_3:ZN", 8.23);

	ink.insert_edge("inst_3:ZN", "inst_1:A1", 0);
	ink.insert_edge("inst_1:A1", "inst_1:ZN", 6.58);
	ink.insert_edge("inst_1:ZN", "inst_0:A3", 0);
	ink.insert_edge("inst_0:A3", "inst_0:ZN", 12.7);

	ink.insert_edge("inst_0:ZN", "inst_15:D", 0);
	ink.insert_edge("inst_0:ZN", "inst_6:A1", 0);
	ink.insert_edge("inst_0:ZN", "inst_12:A", 0);

	ink.insert_edge("inst_6:A1", "inst_6:ZN", 6.8);
	ink.insert_edge("inst_6:ZN", "inst_16:D", 0);

	ink.insert_edge("inst_11:A", "inst_11:ZN", 5.04);
	ink.insert_edge("inst_11:ZN", "inst_6:A2", 0);
	ink.insert_edge("inst_6:A2", "inst_6:ZN", 8.89);

	ink.insert_edge("inst_15:CK", "inst_15:D", 0);


	// G3
	ink.insert_edge("G3", "inst_1:A2", 0);
	ink.insert_edge("inst_1:A2", "inst_1:ZN", 8.75);

	ink.insert_edge("inst_4:A2", "inst_4:ZN", 10.3);
	ink.insert_edge("inst_4:ZN", "inst_2:A1", 0);
	ink.insert_edge("inst_2:A1", "inst_2:ZN", 6.63, 6.62);
	ink.insert_edge("inst_2:ZN", "inst_0:A1", 0);
	ink.insert_edge("inst_0:A1", "inst_0:ZN", 9.22);

	ink.insert_edge("inst_4:A1", "inst_4:ZN", 8.23);


	ink.insert_edge("inst_12:A", "inst_12:ZN", 6.81, 6.82);
	ink.insert_edge("inst_12:ZN", "G17", 0);

	REQUIRE(ink.num_edges() == 32);
	REQUIRE(ink.num_verts() == 29);


	auto paths = ink.report_incsfxt(8);
	
	REQUIRE(paths.size() == 8);

	// 1st path
	// ----------------------------------
	// Startpoint:  inst_15:CK
	// Endpoint:    inst_15:D
	// Path:
	// Vert name: inst_15:CK, Dist: 0
	// Vert name: inst_15:D, Dist: 0
	// ----------------------------------
	REQUIRE(paths[0].size() == 2);
	auto it = paths[0].begin();

	REQUIRE(it->vert.name == "inst_15:CK");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_15:D");
	REQUIRE(float_equal(it->dist, 0));

	// 2nd path
	// ----------------------------------
	// Startpoint:  G0
	// Endpoint:    inst_16:D
	// Path:
	// Vert name: G0, Dist: 0
	// Vert name: inst_11:A, Dist: 0
	// Vert name: inst_11:ZN, Dist: 5.04
	// Vert name: inst_6:A2, Dist: 5.04
	// Vert name: inst_6:ZN, Dist: 13.93
	// Vert name: inst_16:D, Dist: 13.93
	// ----------------------------------
	REQUIRE(paths[1].size() == 6);
	it = paths[1].begin();

	REQUIRE(it->vert.name == "G0");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_11:A");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_11:ZN");
	REQUIRE(float_equal(it->dist, 5.04));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_6:A2");
	REQUIRE(float_equal(it->dist, 5.04));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_6:ZN");
	REQUIRE(float_equal(it->dist, 13.93));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_16:D");
	REQUIRE(float_equal(it->dist, 13.93));

	// 3rd path
	// ----------------------------------
	// Startpoint:  G3
	// Endpoint:    inst_15:D
	// Path:
	// Vert name: G3, Dist: 0
	// Vert name: inst_1:A2, Dist: 0
	// Vert name: inst_1:ZN, Dist: 8.75
	// Vert name: inst_0:A3, Dist: 8.75
	// Vert name: inst_0:ZN, Dist: 21.45
	// Vert name: inst_15:D, Dist: 21.45
	// ----------------------------------
	REQUIRE(paths[2].size() == 6);
	it = paths[2].begin();

	REQUIRE(it->vert.name == "G3");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A2");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A3");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 21.45));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_15:D");
	REQUIRE(float_equal(it->dist, 21.45));

	// 4th path
	// ----------------------------------
	// Startpoint:  G0
	// Endpoint:    inst_15:D
	// Path:
	// Vert name: G0, Dist: 0
	// Vert name: inst_4:A2, Dist: 0
	// Vert name: inst_4:ZN, Dist: 10.3
	// Vert name: inst_2:A1, Dist: 10.3
	// Vert name: inst_2:ZN, Dist: 16.92
	// Vert name: inst_0:A1, Dist: 16.92
	// Vert name: inst_0:ZN, Dist: 26.14
	// Vert name: inst_15:D, Dist: 26.14
	// ----------------------------------
	REQUIRE(paths[3].size() == 8);
	it = paths[3].begin();

	REQUIRE(it->vert.name == "G0");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:A2");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:ZN");
	REQUIRE(float_equal(it->dist, 10.3));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:A1");
	REQUIRE(float_equal(it->dist, 10.3));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:ZN");
	REQUIRE(float_equal(it->dist, 16.92));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A1");
	REQUIRE(float_equal(it->dist, 16.92));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 26.14));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_15:D");
	REQUIRE(float_equal(it->dist, 26.14));

	// 5th path
	// ----------------------------------
	// Startpoint:  G0
	// Endpoint:    inst_15:D
	// Path:
	// Vert name: G0, Dist: 0
	// Vert name: inst_4:A2, Dist: 0
	// Vert name: inst_4:ZN, Dist: 10.3
	// Vert name: inst_2:A1, Dist: 10.3
	// Vert name: inst_2:ZN, Dist: 16.93
	// Vert name: inst_0:A1, Dist: 16.93
	// Vert name: inst_0:ZN, Dist: 26.15
	// Vert name: inst_15:D, Dist: 26.15
	// ----------------------------------

	REQUIRE(paths[4].size() == 8);
	it = paths[4].begin();

	REQUIRE(it->vert.name == "G0");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:A2");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_4:ZN");
	REQUIRE(float_equal(it->dist, 10.3));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:A1");
	REQUIRE(float_equal(it->dist, 10.3));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_2:ZN");
	REQUIRE(float_equal(it->dist, 16.93));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A1");
	REQUIRE(float_equal(it->dist, 16.93));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 26.15));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_15:D");
	REQUIRE(float_equal(it->dist, 26.15));


	// 6th path
	// ----------------------------------
	// Startpoint:  G3
	// Endpoint:    inst_16:D
	// Path:
	// Vert name: G3, Dist: 0
	// Vert name: inst_1:A2, Dist: 0
	// Vert name: inst_1:ZN, Dist: 8.75
	// Vert name: inst_0:A3, Dist: 8.75
	// Vert name: inst_0:ZN, Dist: 21.45
	// Vert name: inst_6:A1, Dist: 21.45
	// Vert name: inst_6:ZN, Dist: 28.25
	// Vert name: inst_16:D, Dist: 28.25
	// ----------------------------------
	REQUIRE(paths[5].size() == 8);
	it = paths[5].begin();

	REQUIRE(it->vert.name == "G3");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A2");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A3");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 21.45));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_6:A1");
	REQUIRE(float_equal(it->dist, 21.45));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_6:ZN");
	REQUIRE(float_equal(it->dist, 28.25));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_16:D");
	REQUIRE(float_equal(it->dist, 28.25));

	// 7th path
	// ----------------------------------
	// Startpoint:  G3
	// Endpoint:    G17
	// Path:
	// Vert name: G3, Dist: 0
	// Vert name: inst_1:A2, Dist: 0
	// Vert name: inst_1:ZN, Dist: 8.75
	// Vert name: inst_0:A3, Dist: 8.75
	// Vert name: inst_0:ZN, Dist: 21.45
	// Vert name: inst_12:A, Dist: 21.45
	// Vert name: inst_12:ZN, Dist: 28.26
	// Vert name: G17, Dist: 28.26
	// ----------------------------------
	REQUIRE(paths[6].size() == 8);
	it = paths[6].begin();

	REQUIRE(it->vert.name == "G3");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A2");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A3");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 21.45));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_12:A");
	REQUIRE(float_equal(it->dist, 21.45));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_12:ZN");
	REQUIRE(float_equal(it->dist, 28.26));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "G17");
	REQUIRE(float_equal(it->dist, 28.26));

	// 8th path
	// ----------------------------------
	// Startpoint:  G3
	// Endpoint:    G17
	// Path:
	// Vert name: G3, Dist: 0
	// Vert name: inst_1:A2, Dist: 0
	// Vert name: inst_1:ZN, Dist: 8.75
	// Vert name: inst_0:A3, Dist: 8.75
	// Vert name: inst_0:ZN, Dist: 21.45
	// Vert name: inst_12:A, Dist: 21.45
	// Vert name: inst_12:ZN, Dist: 28.27
	// Vert name: G17, Dist: 28.27
	// ----------------------------------
	REQUIRE(paths[7].size() == 8);
	it = paths[7].begin();

	REQUIRE(it->vert.name == "G3");
	REQUIRE(float_equal(it->dist, 0));
	
	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:A2");
	REQUIRE(float_equal(it->dist, 0));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_1:ZN");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:A3");
	REQUIRE(float_equal(it->dist, 8.75));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_0:ZN");
	REQUIRE(float_equal(it->dist, 21.45));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_12:A");
	REQUIRE(float_equal(it->dist, 21.45));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "inst_12:ZN");
	REQUIRE(float_equal(it->dist, 28.27));

	std::advance(it, 1);
	REQUIRE(it->vert.name == "G17");
	REQUIRE(float_equal(it->dist, 28.27));


	for (const auto& p : paths) {
		p.dump(std::cout);
	}


}

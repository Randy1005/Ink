#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>
const float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}

TEST_CASE("bound check" * doctest::timeout(300)) {
	ink::Ink ink;
  
  ink.bounds = {3.5, 4.5, 5.5};
  auto q_idx = ink.determine_q_idx(2.2);
  REQUIRE(q_idx == 0);
  q_idx = ink.determine_q_idx(3.5);
  REQUIRE(q_idx == 1);
  q_idx = ink.determine_q_idx(4.5);
  REQUIRE(q_idx == 2);
  q_idx = ink.determine_q_idx(5.5);
  REQUIRE(q_idx == 3);
  q_idx = ink.determine_q_idx(5.8);
  REQUIRE(q_idx == 3);

  ink.bounds = {2.2, 3.3, 4.4, 5.5, 6.6, 7.7};
  q_idx = ink.determine_q_idx(0);
  REQUIRE(q_idx == 0);
  q_idx = ink.determine_q_idx(2.3);
  REQUIRE(q_idx == 1);
  q_idx = ink.determine_q_idx(3.4);
  REQUIRE(q_idx == 2);
  q_idx = ink.determine_q_idx(4.5);
  REQUIRE(q_idx == 3);
  q_idx = ink.determine_q_idx(5.8);
  REQUIRE(q_idx == 4);
  q_idx = ink.determine_q_idx(6.8);
  REQUIRE(q_idx == 5);
  q_idx = ink.determine_q_idx(7.8);
  REQUIRE(q_idx == 6);
}




TEST_CASE("Simple graph (see slides)" * doctest::timeout(300)) {
	ink::Ink ink;
	// e0
  ink.insert_edge("v0", "v1", -1);
	// e1
  ink.insert_edge("v0", "v2", 3);
	// e2
  ink.insert_edge("v2", "v3", 1);
	// e3
  ink.insert_edge("v2", "v4", 2);
	// e4
  ink.insert_edge("v3", "v1", 3);
  // e5
  ink.insert_edge("v1", "v5", 10);
  // e6
  ink.insert_edge("v1", "v6", 2);
  // e14
  ink.insert_edge("v1", "v_new", 9);
  // e7
  ink.insert_edge("v5", "v7", -4);
  // e9
  ink.insert_edge("v6", "v9", 5);
  // e10
  ink.insert_edge("v6", "v10", 2);
  // e13
  ink.insert_edge("v4", "v6", 6);

	ink.report_incsfxt(10, false, false);
  
  auto costs = ink.get_path_costs();

  auto old_max_dc = costs.back();
  ink.report_multiq(old_max_dc, 10, 3, false);
  costs = ink.get_path_costs_from_cq();
  for (auto& c : costs) {
    std::cout << c << '\n';
  }
}



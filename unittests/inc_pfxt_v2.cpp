#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>
const float eps = 0.0001f;
bool float_equal(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}

TEST_CASE("Insert / Remove / Update  Edges (two inc iterations)" * doctest::timeout(300)) {
	ink::Ink ink_inc, inc_full;
	// e0
  ink_inc.insert_edge("v0", "v1", -1);
  inc_full.insert_edge("v0", "v1", -1);
	// e1
  ink_inc.insert_edge("v0", "v2", 3);
  inc_full.insert_edge("v0", "v2", 3);
	// e2
  ink_inc.insert_edge("v2", "v3", 1);
  inc_full.insert_edge("v2", "v3", 1);
	// e3
  ink_inc.insert_edge("v2", "v4", 2);
  inc_full.insert_edge("v2", "v4", 2);
	// e4
  ink_inc.insert_edge("v3", "v1", 3);
  inc_full.insert_edge("v3", "v1", 3);
  // e5
  ink_inc.insert_edge("v1", "v5", 1);
  inc_full.insert_edge("v1", "v5", 1);
  // e6
  ink_inc.insert_edge("v1", "v6", 2);
  inc_full.insert_edge("v1", "v6", 2);
  // e7
  ink_inc.insert_edge("v5", "v7", -4);
  inc_full.insert_edge("v5", "v7", -4);
  // e8
  ink_inc.insert_edge("v5", "v8", 8);
  inc_full.insert_edge("v5", "v8", 8);
  // e9
  ink_inc.insert_edge("v6", "v9", 5);
  inc_full.insert_edge("v6", "v9", 5);
  // e10
  ink_inc.insert_edge("v6", "v10", 7);
  inc_full.insert_edge("v6", "v10", 7);
  // e11
  ink_inc.insert_edge("v8", "v11", 4);
  inc_full.insert_edge("v8", "v11", 4);
  // e12
  ink_inc.insert_edge("v8", "v12", 11);
  inc_full.insert_edge("v8", "v12", 11);
	
	// report once with incsfxt, save the pfxt nodes
	ink_inc.report_incsfxt(4, true, false);
  auto costs_inc = ink_inc.get_path_costs();
  inc_full.report_incsfxt(4, false, false);
  auto costs_full = inc_full.get_path_costs();
  
  
  // iteration 1
  // insert / remove edges
  ink_inc.remove_edge("v5", "v8");  
  inc_full.remove_edge("v5", "v8");  
  
  ink_inc.insert_edge("v9", "v8", 3);
  inc_full.insert_edge("v9", "v8", 3);
  
  ink_inc.insert_edge("v1", "v_new", 9);  
  inc_full.insert_edge("v1", "v_new", 9);  
  
  // update edges
  ink_inc.insert_edge("v1", "v5", 10);
  inc_full.insert_edge("v1", "v5", 10);
  
  ink_inc.report_incremental_v2(10, true, false);   
  costs_inc = ink_inc.get_path_costs();

  inc_full.report_incsfxt(10, false, false);   
  costs_full = inc_full.get_path_costs();
  REQUIRE(costs_full.size() == 10);

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_inc[i], costs_full[i]));
  }

  // iteration 2
  // re-connect v5 to v8
  ink_inc.insert_edge("v5", "v8", 2);  
  inc_full.insert_edge("v5", "v8", 2);  
  
  ink_inc.report_incremental_v2(10, true, false);   
  costs_inc = ink_inc.get_path_costs();

  inc_full.report_incsfxt(10, false, false);   
  costs_full = inc_full.get_path_costs();
  REQUIRE(costs_full.size() == 10);

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_inc[i], costs_full[i]));
  }
  

}


TEST_CASE("Linear chains (with buffer insertions)" * doctest::timeout(300)) { 
  ink::Ink ink_full;
  ink::Ink ink_inc;

  // first chain
  ink_full.insert_edge("S", "ch0-0", 1);
  ink_inc.insert_edge("S", "ch0-0", 1);
  
  ink_full.insert_edge("ch0-0", "ch0-1", 2);
  ink_inc.insert_edge("ch0-0", "ch0-1", 2);

  ink_full.insert_edge("ch0-1", "ch0-2", 3);
  ink_inc.insert_edge("ch0-1", "ch0-2", 3);
  
  // second chain
  ink_full.insert_edge("S", "ch1-0", 4);
  ink_inc.insert_edge("S", "ch1-0", 4);
  
  ink_full.insert_edge("ch1-0", "ch1-1", 2);
  ink_inc.insert_edge("ch1-0", "ch1-1", 2);

  ink_full.insert_edge("ch1-1", "ch1-2", 4);
  ink_inc.insert_edge("ch1-1", "ch1-2", 4);
  
  // third chain
  ink_full.insert_edge("S", "ch2-0", 12);
  ink_inc.insert_edge("S", "ch2-0", 12);
  
  ink_full.insert_edge("ch2-0", "ch2-1", 2);
  ink_inc.insert_edge("ch2-0", "ch2-1", 2);

  ink_full.insert_edge("ch2-1", "ch2-2", 8);
  ink_inc.insert_edge("ch2-1", "ch2-2", 8);
  
  // report paths
  ink_full.report_incsfxt(3, false, false);
  auto costs_full = ink_full.get_path_costs();
  ink_inc.report_incsfxt(3, true, false);
  auto costs_inc = ink_inc.get_path_costs();

  // chain 0: insert buffers
  auto& e1 = ink_full.get_edge("S", "ch0-0");
  ink_full.insert_buffer("BUF.S->ch0-0", e1);
  
  auto& e2 = ink_inc.get_edge("S", "ch0-0");
  ink_inc.insert_buffer("BUF.S->ch0-0", e2);

  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();
  
  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  // chain 0: insert buffers
  auto& e3 = ink_full.get_edge("ch0-0", "ch0-1");
  ink_full.insert_buffer("BUF.ch0-0->ch0-1", e3);
  
  auto& e4 = ink_inc.get_edge("ch0-0", "ch0-1");
  ink_inc.insert_buffer("BUF.ch0-0->ch0-1", e4);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  // chain 0: insert buffers
  auto& e5 = ink_full.get_edge("ch0-1", "ch0-2");
  ink_full.insert_buffer("BUF.ch0-1->ch0-2", e5);
  
  auto& e6 = ink_inc.get_edge("ch0-1", "ch0-2");
  ink_inc.insert_buffer("BUF.ch0-1->ch0-2", e6);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }
  
  // chain 1: insert buffers
  auto& e7 = ink_full.get_edge("S", "ch1-0");
  ink_full.insert_buffer("BUF.S->ch1-0", e7);
  
  auto& e8 = ink_inc.get_edge("S", "ch1-0");
  ink_inc.insert_buffer("BUF.S->ch1-0", e8);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  // chain 1: insert buffers
  auto& e9 = ink_full.get_edge("ch1-0", "ch1-1");
  ink_full.insert_buffer("BUF.ch1-0->ch1-1", e9);
  
  auto& e10 = ink_inc.get_edge("ch1-0", "ch1-1");
  ink_inc.insert_buffer("BUF.ch1-0->ch1-1", e10);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }
  
  // chain 1: insert buffers
  auto& e11 = ink_full.get_edge("ch1-1", "ch1-2");
  ink_full.insert_buffer("BUF.ch1-1->ch1-2", e11);
  
  auto& e12 = ink_inc.get_edge("ch1-1", "ch1-2");
  ink_inc.insert_buffer("BUF.ch1-1->ch1-2", e12);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  // chain 2: insert buffers
  auto& e13 = ink_full.get_edge("S", "ch2-0");
  ink_full.insert_buffer("BUF.S->ch2-0", e13);
  
  auto& e14 = ink_inc.get_edge("S", "ch2-0");
  ink_inc.insert_buffer("BUF.S->ch2-0", e14);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  // chain 2: insert buffers
  auto& e15 = ink_full.get_edge("ch2-0", "ch2-1");
  ink_full.insert_buffer("BUF.ch2-0->ch2-1", e15);
  
  auto& e16 = ink_inc.get_edge("ch2-0", "ch2-1");
  ink_inc.insert_buffer("BUF.ch2-0->ch2-1", e16);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  // chain 2: insert buffers
  auto& e17 = ink_full.get_edge("ch2-1", "ch2-2");
  ink_full.insert_buffer("BUF.ch2-1->ch2-2", e17);
  
  auto& e18 = ink_inc.get_edge("ch2-1", "ch2-2");
  ink_inc.insert_buffer("BUF.ch2-1->ch2-2", e18);
  
  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  // remove buffers in arbitrary order and report paths
  ink_full.remove_buffer("BUF.ch1-0->ch1-1");
  ink_inc.remove_buffer("BUF.ch1-0->ch1-1");

   // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  ink_full.remove_buffer("BUF.ch2-0->ch2-1");
  ink_inc.remove_buffer("BUF.ch2-0->ch2-1");

   // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  ink_full.remove_buffer("BUF.S->ch1-0");
  ink_inc.remove_buffer("BUF.S->ch1-0");

   // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  ink_full.remove_buffer("BUF.ch1-1->ch1-2");
  ink_inc.remove_buffer("BUF.ch1-1->ch1-2");

  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }

  ink_full.remove_buffer("BUF.S->ch2-0");
  ink_inc.remove_buffer("BUF.S->ch2-0");

  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }
   
  ink_full.remove_buffer("BUF.ch2-1->ch2-2");
  ink_inc.remove_buffer("BUF.ch2-1->ch2-2");

  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }
  
  ink_full.remove_buffer("BUF.S->ch0-0");
  ink_inc.remove_buffer("BUF.S->ch0-0");

  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }
  
  ink_full.remove_buffer("BUF.ch0-0->ch0-1");
  ink_inc.remove_buffer("BUF.ch0-0->ch0-1");

  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }
  
  ink_full.remove_buffer("BUF.ch0-1->ch0-2");
  ink_inc.remove_buffer("BUF.ch0-1->ch0-2");

  // report again
  ink_full.report_incsfxt(3, false, false);
  costs_full = ink_full.get_path_costs();
  ink_inc.report_incremental_v2(3, true, false);
  costs_inc = ink_inc.get_path_costs();

  for (size_t i = 0; i < costs_full.size(); i++) {
    REQUIRE(float_equal(costs_full[i], costs_inc[i]));
  }
  
  for (auto c : costs_inc) {
    std::cout << c << '\n';
  }
}

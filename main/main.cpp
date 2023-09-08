#include <ink/ink.hpp>


//std::vector<ink::Path> report_N(ink::Ink& ink, size_t N, size_t K, size_t mode) {
//	size_t elapsed{0};
//	
//	std::vector<ink::Path> paths;
//	for (size_t i = 0; i < N; i++) {
//		ink.insert_edge("x10842", "inst_83400:RN", 0.00245);	
//		ink.insert_edge("x10842", "inst_83497:RN", 0, -0.00032);	
//			
//		auto beg = std::chrono::steady_clock::now();
//		if (mode == 0) {
//			paths = ink.report_incsfxt(K, false);	
//		}
//		else if (mode == 1) {
//			paths = ink.report_incremental(K, false, true);
//		}
//		auto end = std::chrono::steady_clock::now();
//		elapsed +=
//			std::chrono::duration_cast<std::chrono::milliseconds>(end-beg).count();
//		std::cout << "report " 
//							<< i << " finished, accumulated elapsed = " 
//							<< elapsed << " ms.\n";
//		ink.dump_profile(std::cout, true);
//	}
//
//	std::cout << "accumulated report time of " 
//						<< N << " reports (" << K << " paths per report) = "
//						<< elapsed << " ms. (" << elapsed / 1000.0f << " s)\n";
//	return paths;
//}


int main(int argc, char* argv[]) {
	if (argc < 5) {
		std::cerr 
			<< "usage: ./Ink [graph_ops_file] [output_file] [num_paths] [mode] {opt: percent to update}\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_ops(argv[1], argv[2]);

	size_t num_paths = std::stoul(argv[3]);
	size_t mode = std::stoi(argv[4]);

	std::cout << "num_paths = " << num_paths << '\n';
	std::cout << "report_mode = ";
	if (mode == 0) {
		std::cout << "original\n";
	}
	else if (mode == 1) {
		std::cout << "incpfxt\n";
	}
  else {
    std::cout << "percentage-update\n";
  }

	// after reading all the edges
	// we then report using 2 different mode
	if (mode == 0) {
    auto costs_old = ink.get_path_costs();
    ink.dump_profile(std::cout, true);

    // report again
    // disable save_pfxt_nodes
		ink.report_incsfxt(num_paths, false, false);
		ink.dump_profile(std::cout, true);

    std::ofstream ofs(argv[2]);
		// output costs to a file
    auto costs_new = ink.get_path_costs();
    for (size_t i = 0; i < num_paths; i++) {
      if (i > costs_new.size() - 1) {
        break;
      }

			ofs << costs_new[i] << '\n';
		}

    size_t diff = 0;
    // count how many different costs between old & new
    for (size_t i = 0; i < costs_old.size(); i++) {
      if (costs_old[i] != costs_new[i]) {
        diff++;
      }
    }

    std::cout << "diff = " << diff << " / " << costs_old.size() << '\n';
	
	}
	else if (mode == 1) {
    auto costs_old = ink.get_path_costs();
		ink.dump_profile(std::cout, true);
	  	
    // report again with incremental pfxt
		// disable save_pfxt_nodes
		ink.report_incremental(num_paths, false, false, false);
		ink.dump_profile(std::cout, true);
		
    std::ofstream ofs(argv[2]);
		// output costs to a file
    auto costs_new = ink.get_path_costs();
    for (size_t i = 0; i < num_paths; i++) {
      if (i > costs_new.size() - 1) {
        break;
      }
			ofs << costs_new[i] << '\n';
		}

    size_t diff = 0;
    // count how many different costs between old & new
    for (size_t i = 0; i < costs_old.size(); i++) {
      if (costs_old[i] != costs_new[i]) {
        diff++;
      }
    }

    std::cout << "diff = " << diff << " / " << costs_old.size() << '\n';
	}
  else {
    // 2 instances of Ink
    ink::Ink i1, i2;
    
    std::string i1_out = std::string(argv[2]) + "-orig";
    std::string i2_out = std::string(argv[2]) + "-incpfxt";
	  i1.read_ops(argv[1], i1_out);
	  i2.read_ops(argv[1], i2_out);

    auto perc = std::stof(argv[5]);

    i1.report_incsfxt(num_paths, true, false);
    auto c1 = i1.get_path_costs();
    i1.update_edges_percent(perc, "i1-dmp"); 
		i1.report_incsfxt(num_paths, false, false);
		auto spur_time_i1 = i1.elapsed_time_spur;

    i2.report_incsfxt(num_paths, true, false);  
    i2.update_edges_percent(perc, "i2-dmp");  
		i2.report_incremental(num_paths, false, false, false);
		auto spur_time_i2 = i2.elapsed_time_spur;
    
    std::ofstream ofs1(i1_out);
    std::ofstream ofs2(i2_out);
		// output costs to a file
    auto i1_costs = i1.get_path_costs();
    size_t diff = 0;
    for (size_t i = 0; i < num_paths; i++) {
      if (i > i1_costs.size() - 1) {
        break;
      }
			ofs1 << i1_costs[i] << '\n';

      if (c1[i] != i1_costs[i]) {
        diff++;
      }
		}
    
    auto i2_costs = i2.get_path_costs();
    for (size_t i = 0; i < num_paths; i++) {
      if (i > i2_costs.size() - 1) {
        break;
      }
			ofs2 << i2_costs[i] << '\n';
		}

    std::cout << "orig spur time=" << spur_time_i1 << '\n';
    std::cout << "incpfxt spur time=" << spur_time_i2 << '\n';
    auto speedup = spur_time_i1 / static_cast<float>(spur_time_i2);
    std::cout << "speedup=" << speedup << '\n';
    std::cout << "diff=" << diff << '\n';
  }
	
	return 0;
}



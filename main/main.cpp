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
	if (argc != 5) {
		std::cerr 
			<< "usage: ./Ink [graph_ops_file] [output_file] [num_paths] [mode]\n";
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
		std::cout << "+incpfxt\n";
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
	
	return 0;
}



//int main(int argc, char* argv[]) {
//
//	ink::Ink ink;
//	ink.read_ops(argv[1], argv[2]);
//	size_t num_paths = std::stoul(argv[3]);
//	
//	// 10 design modifiers, we wish to compare the similarity of
//	// the path reports
//	for (size_t i = 0; i < 10; i++) {
//		auto paths_a = ink.report_incsfxt(num_paths);
//		ink.modify_random_vertex();
//		auto paths_b = ink.report_incsfxt(num_paths);
//		
//		std::vector<float> diff_vec;
//		float diff = ink.vec_diff(paths_a, paths_b, diff_vec);
//		std::cout << "diff size=" << diff_vec.size() << '\n';
//		std::cout << "query diff=" << diff << '\n';
//	}
//}

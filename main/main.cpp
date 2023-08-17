#include <ink/ink.hpp>


std::vector<ink::Path> report_N(ink::Ink& ink, size_t N, size_t K, size_t mode) {
	size_t elapsed{0};
	
	std::vector<ink::Path> paths;
	for (size_t i = 0; i < N; i++) {
		ink.insert_edge("x10842", "inst_83400:RN", 0.00245);	
		ink.insert_edge("x10842", "inst_83497:RN", 0, -0.00032);	
			
		auto beg = std::chrono::steady_clock::now();
		if (mode == 0) {
			paths = ink.report_incsfxt(K, false);	
		}
		else if (mode == 1) {
			paths = ink.report_incremental(K, false, true);
		}
		auto end = std::chrono::steady_clock::now();
		elapsed +=
			std::chrono::duration_cast<std::chrono::milliseconds>(end-beg).count();
		std::cout << "report " 
							<< i << " finished, accumulated elapsed = " 
							<< elapsed << " ms.\n";
		ink.dump_profile(std::cout, true);
	}

	std::cout << "accumulated report time of " 
						<< N << " reports (" << K << " paths per report) = "
						<< elapsed << " ms. (" << elapsed / 1000.0f << " s)\n";
	return paths;
}


int main(int argc, char* argv[]) {
	if (argc != 6) {
		std::cerr 
			<< "usage: ./Ink [graph_ops_file] [output_file] [num_paths] [mode] [reports]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_ops(argv[1], argv[2]);

	size_t num_paths = std::stoul(argv[3]);
	size_t mode = std::stoi(argv[4]);
	size_t reports = std::stoul(argv[5]);

	std::cout << "num_paths = " << num_paths << '\n';
	std::cout << "report_mode = ";
	if (mode == 0) {
		std::cout << "+incsfxt\n";
	}
	else if (mode == 1) {
		std::cout << "+incpfxt\n";
	}
	else if (mode == 2) {
		std::cout << "redo-all\n";
	}
	std::cout << "num_reports = " << reports << '\n';

	// after reading all the edges
	// we then report using 2 different mode
	
	ink.dump_profile(std::cout, true);
	if (mode == 0) {
		// report again with incremental sfxt
		auto paths = ink.report_incsfxt(num_paths, true);
		ink.dump_profile(std::cout, true);
		//auto paths = report_N(ink, reports, num_paths, mode);

		
		std::ofstream ofs(argv[2]);
		// output paths to a file
		ofs << paths.size() << '\n';
		for (auto& p : paths) {
			ofs << p.weight << '\n';
		}
	
		// dump pfxt nodes to a file
		ofs = std::ofstream("incsfxt.nodes");
		ink.dump_pfxt_nodes(ofs);
	
	}
	else if (mode == 1) {
		// report again with incremental pfxt
		// disable save_pfxt_nodes
		// enable use_leaders
		auto paths = ink.report_incremental(num_paths, true, true);
		ink.dump_profile(std::cout, true);
		//auto paths = report_N(ink, reports, num_paths, mode);
		
		std::ofstream ofs(argv[2]);
		// output paths to a file
		ofs << paths.size() << '\n';
		for (auto& p : paths) {
			ofs << p.weight << '\n';
		}
	
	
		// dump pfxt nodes to a file
		ofs = std::ofstream("incpfxt.nodes");
		ink.dump_pfxt_nodes(ofs);
	
	}
	else if (mode == 2) {
		// report again with sfxt/pfxt rebuilt
		auto paths = ink.report_rebuild(num_paths);
		ink.dump_profile(std::cout, true);
		
		std::ofstream ofs(argv[2]);
		// output paths to a file
		ofs << paths.size() << '\n';
		for (auto& p : paths) {
			ofs << p.weight << '\n';
		}
	}
	else if (mode == 3) {
		std::ofstream ofs("pfxt_srcs.out");
		ink.dump_pfxt_srcs(ofs);
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

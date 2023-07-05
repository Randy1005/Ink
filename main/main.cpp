#include <ink/ink.hpp>

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
		std::cout << "+incsfxt\n";
	}
	else if (mode == 1) {
		std::cout << "+incpfxt\n";
	}
	else if (mode == 2) {
		std::cout << "redo-all\n";
	}

	// after reading all the edges
	// we then report using 2 different mode
	if (mode == 0) {
		ink.dump_profile(std::cout, true);
		// report again with incremental sfxt
		auto paths = ink.report_incsfxt(num_paths);
		ink.dump_profile(std::cout, true);
		
		std::ofstream ofs(argv[2]);
		// output paths to a file
		ofs << paths.size() << '\n';
		for (auto& p : paths) {
			ofs << p.weight << '\n';
		}
	}
	else if (mode == 1) {
		ink.dump_profile(std::cout, true);

		// report again with incremental pfxt
		// disable save_pfxt_nodes
		// enable use_leaders
		auto paths = ink.report_incremental(num_paths, false, true);
		ink.dump_profile(std::cout, true);
		
		std::ofstream ofs(argv[2]);
		// output paths to a file
		ofs << paths.size() << '\n';
		for (auto& p : paths) {
			ofs << p.weight << '\n';
		}
	}
	else if (mode == 2) {
		ink.dump_profile(std::cout, true);
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

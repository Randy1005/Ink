#include <ink/ink.hpp>

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
		std::cout << "incsfxt\n";
	}
	else if (mode == 1) {
		std::cout << "incsfxt + incpfxt\n";
	}
  else if (mode == 2) {
    std::cout << "full\n";
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
	
	}
	else if (mode == 1) {
    auto costs_old = ink.get_path_costs();
		//ink.dump_profile(std::cout, true);
	  	
    // report again with incremental pfxt
		// disable save_pfxt_nodes
		ink.report_incremental(num_paths, false, false, false);
		//ink.dump_profile(std::cout, true);
		
    std::ofstream ofs(argv[2]);
		// output costs to a file
    auto costs_new = ink.get_path_costs();
    for (size_t i = 0; i < num_paths; i++) {
      if (i > costs_new.size() - 1) {
        break;
      }
			ofs << costs_new[i] << '\n';
		}
	}
  else if (mode == 2) {
    auto costs_old = ink.get_path_costs();
	  	
    // report again with everything rebuilt 
		ink.report_rebuild(num_paths, false);
		
    std::ofstream ofs(argv[2]);
		// output costs to a file
    auto costs_new = ink.get_path_costs();
    for (size_t i = 0; i < num_paths; i++) {
      if (i > costs_new.size() - 1) {
        break;
      }
			ofs << costs_new[i] << '\n';
		}

  }
  else {
    // 2 instances of Ink
    ink::Ink i1, i2;
    
    std::string i1_out = std::string(argv[2]) + "-full";
    std::string i2_out = std::string(argv[2]) + "-inc";
	  i1.read_ops(argv[1], i1_out);
	  i2.read_ops(argv[1], i2_out);

    auto perc = std::stof(argv[5]);

    i1.report_incsfxt(num_paths, true, false);
    i1.update_edges_percent(perc); 
		i1.report_rebuild(num_paths, false);
    auto c1 = i1.get_path_costs();

    i2.report_incsfxt(num_paths, true, false);  
    i2.update_edges_percent(perc);  
		i2.report_incremental(num_paths, false, false, false);
    auto c2 = i2.get_path_costs();
    
    std::cout << "full sfxt + pfxt time=" << i1.full_time << '\n';
    std::cout << "inc sfxt + pfxt time=" << i2.incremental_time << '\n';
    auto speedup = i1.full_time / static_cast<float>(i2.incremental_time);
    std::cout << "speedup=" << speedup << '\n';
  
    std::ofstream ofs1(i1_out), ofs2(i2_out);
    for (size_t i = 0; i < c1.size(); i++) {
      ofs1 << c1[i] << '\n';
    }
  
    for (size_t i = 0; i < c2.size(); i++) {
      ofs2 << c2[i] << '\n';
    }
  }
	
	return 0;
}



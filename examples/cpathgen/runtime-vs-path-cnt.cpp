#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "usage: ./a.out [input] [golden]\n";
    std::exit(EXIT_FAILURE);
  }

  // strip input graph filename from the directory
  std::string filename = argv[1];
  size_t last_slash = filename.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    filename = filename.substr(last_slash+1);
  }

  // also strip the suffix
  size_t dot_pos = filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    filename = filename.substr(0, dot_pos);
  } 

  ink::Ink ink_ot, ink_pathgen, ink_cpathgen;
  std::ifstream ifs(argv[2]);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open golden file: " << argv[3] << '\n';
    return EXIT_FAILURE;
  }
  std::vector<float> golden_costs;
  float cost;

  #pragma omp parallel
  #pragma omp single
  {
    #pragma omp task
    ink_ot.read_ops(argv[1], filename+".ot");
    #pragma omp task
    ink_pathgen.read_ops(argv[1], filename+".pathgen");
    #pragma omp task
    ink_cpathgen.read_ops(argv[1], filename+".cpathgen");
    #pragma omp task
    {
      while (ifs >> cost) {
        golden_costs.push_back(cost);
      }
      ifs.close();
    }
  }

  // write header
  std::ofstream ofs_combined("runtime-vs-path-cnt."+filename+".combined.csv");
  ofs_combined << "path_count,pfxt_time_ot,pathgen_t8,pathgen_t12,pathgen_t16,cpathgen_t8,cpathgen_t12,cpathgen_t16\n";
  // ofs_cpathgen_t20 << "path_count,cpathgen_t20\n";

  // measure runtime of ot, pathgen w/ 8,12,16 workers, cpathgen w/ 8,12,16 workers
  // at path counts 10, 100, 1000, 10000, 100000, 1000000
  for (size_t path_cnt = 10; path_cnt <= 1000000; path_cnt *= 10) {
    std::cout << "Running for path count: " << path_cnt << '\n';
    
    float cpathgen_t8 = 0, cpathgen_t12 = 0, cpathgen_t16 = 0;
    float pathgen_t8 = 0, pathgen_t12 = 0, pathgen_t16 = 0;
    // cpathgen
    for (size_t num_workers : {8, 12, 16}) {
      ink_cpathgen.report_paths_mlq(
        2.0f,
        path_cnt,
        20,
        std::make_optional<size_t>(num_workers));

      if (num_workers == 8) cpathgen_t8 = ink_cpathgen.pfxt_time/1ms;
      else if (num_workers == 12) cpathgen_t12 = ink_cpathgen.pfxt_time/1ms;
      else if (num_workers == 16) cpathgen_t16 = ink_cpathgen.pfxt_time/1ms;

      std::cout << "CPathGen done for " << num_workers << " threads.\n";
      ink_cpathgen.reset(); 

      ink_pathgen.set_num_workers(num_workers);
      ink_pathgen.set_dequeue_bulk_size(1);
      ink_pathgen.report_multiq(
        golden_costs[path_cnt-1],
        golden_costs.front(),
        path_cnt,
        80,
        false,
        true,
        false);
      if (num_workers == 8) pathgen_t8 = ink_pathgen.pfxt_time/1ms;
      else if (num_workers == 12) pathgen_t12 = ink_pathgen.pfxt_time/1ms;
      else if (num_workers == 16) pathgen_t16 = ink_pathgen.pfxt_time/1ms;

      ink_pathgen.reset();
      std::cout << "PathGen done for " << num_workers << " threads.\n";
    }
    // write to combined file
    ofs_combined << path_cnt << ',' 
                 << std::fixed << std::setprecision(2)
                 << ink_ot.pfxt_time/1ms << ','
                 << pathgen_t8 << ','
                 << pathgen_t12 << ','
                 << pathgen_t16 << ','
                 << cpathgen_t8 << ','
                 << cpathgen_t12 << ','
                 << cpathgen_t16 << '\n';
    ofs_combined.flush();
  }

  
  return 0;
}
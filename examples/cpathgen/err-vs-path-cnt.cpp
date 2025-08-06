#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "usage: ./a.out [input] [golden]\n";
    return EXIT_FAILURE;
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
    std::cerr << "Failed to open golden file: " << argv[2] << '\n';
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

  std::ofstream ofs_combined("err-vs-path-cnt." + filename + ".combined.csv");

  ofs_combined << "k";
  for (const std::string alg : {"pathgen", "cpathgen"}) {
    for (int t : {8, 12, 16}) {
      ofs_combined << ",avg_err_" << alg << "_t" << t
                   << ",max_err_" << alg << "_t" << t
                   << ",min_err_" << alg << "_t" << t;
    }
  }
  ofs_combined << "\n";

  for (size_t k = 10; k <= 1000000; k *= 10) {
    std::cout << "Running for path count: " << k << '\n';

    std::vector<float> avg_err_pathgen, max_err_pathgen, min_err_pathgen;
    std::vector<float> avg_err_cpathgen, max_err_cpathgen, min_err_cpathgen;

    // pathgen: Run for 8, 12, 16 threads
    for (int num_threads : {8, 12, 16}) {
      std::vector<float> errs;
      for (int run = 0; run < 10; ++run) {
        ink_pathgen.set_num_workers(num_threads);
        ink_pathgen.set_dequeue_bulk_size(1);
        ink_pathgen.report_multiq(
          golden_costs[k-1],
          golden_costs.front(),
          k,
          80,
          false,
          true,
          false
        );
        auto pathgen_costs = ink_pathgen.get_path_costs_from_cq();

        float avg_err{0.0f};
        for (size_t i = 0; i < k; ++i) {
          avg_err += std::abs(pathgen_costs[i] - golden_costs[i]) / golden_costs[i];
        }
        avg_err /= k;
        errs.push_back(avg_err * 100.0f);
        ink_pathgen.reset();
      }
      float sum = std::accumulate(errs.begin(), errs.end(), 0.0f);
      avg_err_pathgen.push_back(sum / errs.size());
      max_err_pathgen.push_back(*std::max_element(errs.begin(), errs.end()));
      min_err_pathgen.push_back(*std::min_element(errs.begin(), errs.end()));
      std::cout << "PathGen done for " << num_threads << " threads.\n";
    }

    // cpathgen: Run for 8, 12, 16 threads
    for (int num_threads : {8, 12, 16}) {
      std::vector<float> errs;
      for (int run = 0; run < 10; ++run) {
        ink_cpathgen.report_paths_mlq(
          2.0f, // delta
          k,    // K
          20,   // num_queues
          std::make_optional<size_t>(num_threads)
        );
        auto cpathgen_costs = ink_cpathgen.get_path_costs_from_tbb_cv();
        float avg_err = 0.0f;
        for (size_t i = 0; i < k; ++i) {
          avg_err += std::abs(cpathgen_costs[i] - golden_costs[i]) / golden_costs[i];
        }
        avg_err /= k;
        errs.push_back(avg_err * 100.0f);
        ink_cpathgen.reset();
      }
      float sum = std::accumulate(errs.begin(), errs.end(), 0.0f);
      avg_err_cpathgen.push_back(sum / errs.size());
      max_err_cpathgen.push_back(*std::max_element(errs.begin(), errs.end()));
      min_err_cpathgen.push_back(*std::min_element(errs.begin(), errs.end()));
      std::cout << "CPathGen done for " << num_threads << " threads.\n";
    }

    // Write to combined CSV
    ofs_combined << k;
    for (float val : avg_err_pathgen) {
      ofs_combined << "," << val;
    }
    for (float val : max_err_pathgen) {
      ofs_combined << "," << val;
    }
    for (float val : min_err_pathgen) {
      ofs_combined << "," << val;
    }
    for (float val : avg_err_cpathgen) {
      ofs_combined << "," << val;
    }
    for (float val : max_err_cpathgen) {
      ofs_combined << "," << val;
    }
    for (float val : min_err_cpathgen) {
      ofs_combined << "," << val;
    }
    ofs_combined << '\n';
    ofs_combined.flush();
  }


  return 0;

} 
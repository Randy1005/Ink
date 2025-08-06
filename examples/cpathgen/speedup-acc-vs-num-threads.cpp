#include <ink/ink.hpp>

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "usage: ./a.out [k] [input] [golden]\n";
    std::exit(EXIT_FAILURE);
  }

  size_t k = std::stoi(argv[1]);
  ink::Ink ink_ot, ink_pathgen, ink_cpathgen;

  // strip input graph filename from the directory
  std::string filename = argv[2];
  size_t last_slash = filename.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    filename = filename.substr(last_slash+1);
  }

  // also strip the suffix
  size_t dot_pos = filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    filename = filename.substr(0, dot_pos);
  }


  #pragma omp parallel
  #pragma omp single
  {
    #pragma omp task
    ink_ot.read_ops(argv[2], filename+".ot");
    #pragma omp task
    ink_pathgen.read_ops(argv[2], filename+".pathgen");
    #pragma omp task
    ink_cpathgen.read_ops(argv[2], filename+".cpathgen");
  } 

  // read golden costs
  std::ifstream ifs(argv[3]);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open golden file: " << argv[3] << '\n';
    return EXIT_FAILURE;
  }
  std::vector<float> golden_costs;
  float cost;
  while (ifs >> cost) {
    golden_costs.push_back(cost);
  }
  ifs.close();

  std::chrono::duration<double, std::micro> total_pfxt_time{0};
  const int runs{5};

  // ot (only runtime, accuracy is exact)
  ink_ot.report_incsfxt(k, false, false);
  std::cout << "OT done.\n";

  total_pfxt_time = std::chrono::duration<double, std::micro>{0};
  
  
  // Loop through different numbers of workers
  ink_pathgen.set_dequeue_bulk_size(1);
  ink_pathgen.policy = ink::PartitionPolicy::EQUAL;
  ink_pathgen.overgrow_scalar = 1.0f;
  for (size_t num_workers : {8, 12, 16, 20}) {
    // pathgen (runtime and accuracy)
    auto max_cost = golden_costs[k-1];
    auto min_cost = golden_costs.front();
    ink_pathgen.set_num_workers(num_workers);
    total_pfxt_time = std::chrono::duration<double, std::micro>{0};
    for (int i = 0; i < runs; i++) {
      ink_pathgen.report_multiq(
        max_cost,
        min_cost,
        k,
        80,
        false,
        true,
        false);
      total_pfxt_time += ink_pathgen.pfxt_time;
      std::cout << "run " << i+1 << " done for " << num_workers << " threads.\n";
      if (i < runs-1) {
        ink_pathgen.reset();
      }
    }
    auto pathgen_avg_pfxt_time = total_pfxt_time/runs/1ms;

    // compute max and average error
    auto pathgen_costs = ink_pathgen.get_path_costs_from_cq();
    float pathgen_max_error = 0.0f;
    float pathgen_avg_error = 0.0f;
    for (size_t i = 0; i < k; i++) {
      float error = std::abs(pathgen_costs[i]-golden_costs[i])/golden_costs[i];
      pathgen_max_error = std::max(pathgen_max_error, error);
      pathgen_avg_error += error;
    }
    pathgen_avg_error /= k;
    std::cout << "PathGen done for " << num_workers << " threads.\n";
    ink_pathgen.reset();

    // cpathgen (runtime and accuracy)
    total_pfxt_time = std::chrono::duration<double, std::micro>{0};
    for (int i = 0; i < runs; i++) {
      ink_cpathgen.report_paths_mlq(
        2.0f,
        k,
        20,
        std::make_optional<size_t>(num_workers));
      total_pfxt_time += ink_cpathgen.pfxt_time;
      if (i < runs-1) {
        ink_cpathgen.reset();
      }
    }
    auto cpathgen_avg_pfxt_time = total_pfxt_time/runs/1ms;

    // compute max and average error
    auto cpathgen_costs = ink_cpathgen.get_path_costs_from_tbb_cv();
    float cpathgen_max_error = 0.0f;
    float cpathgen_avg_error = 0.0f;
    for (size_t i = 0; i < k; i++) {
      float error = std::abs(cpathgen_costs[i]-golden_costs[i])/golden_costs[i];
      cpathgen_max_error = std::max(cpathgen_max_error, error);
      cpathgen_avg_error += error;
    }
    cpathgen_avg_error /= k;
    std::cout << "CPathGen done for " << num_workers << " threads.\n";
    ink_cpathgen.reset();

    // if empty, write header
    // header is:
    // num_threads, pathgen_speedup, pathgen_avg_acc, pathgen_max_acc  
    // cpathgen_speedup, avg_accuracy, max_accuracy
    // output to csv
    std::ofstream ofs;
    ofs.open("speedup-acc-vs-num-threads."+filename+".csv", std::ios::app);
    if (ofs.tellp() == 0) {
      ofs << "num_threads,pathgen_speedup,pathgen_avg_err,pathgen_max_err,"
          << "cpathgen_speedup,cpathgen_avg_err,cpathgen_max_err\n";
    }

    auto ot_avg_pfxt_time = ink_ot.pfxt_time/1ms;
    ofs << num_workers << ','
        << std::fixed << std::setprecision(2)
        << (ot_avg_pfxt_time/pathgen_avg_pfxt_time) << ','
        << pathgen_avg_error*100.0f << ','
        << pathgen_max_error*100.0f << ','
        << (ot_avg_pfxt_time/cpathgen_avg_pfxt_time) << ','
        << cpathgen_avg_error*100.0f << ','
        << cpathgen_max_error*100.0f << '\n';

    ofs.flush();
  }
  
  return 0;
}
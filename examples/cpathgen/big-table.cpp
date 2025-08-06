#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "usage: ./a.out [k] [input] [golden]\n";
    return EXIT_FAILURE;
  }

  ink::Ink ot, pathgen, cpathgen;
  size_t k = std::stoi(argv[1]);

  // strip the directory from the input file
  std::string filename = argv[2];
  size_t last_slash = filename.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    filename = filename.substr(last_slash+1);
  }

  std::vector<float> golden_costs;

  #pragma omp parallel
  #pragma omp single
  {
    #pragma omp task
    ot.read_ops(argv[2], filename+".ot");
    #pragma omp task
    pathgen.read_ops(argv[2], filename+".pathgen");
    #pragma omp task
    cpathgen.read_ops(argv[2], filename+".cpathgen");
    #pragma omp task
    {
      // read golden costs
      std::ifstream ifs(argv[3]);
      if (!ifs.is_open()) {
        std::cerr << "Failed to open golden file: " << argv[3] << '\n';
        std::exit(EXIT_FAILURE);
      }
      float cost;
      while (ifs >> cost) {
        golden_costs.push_back(cost);
      }

    }
  }


  std::chrono::duration<double, std::micro> total_sfxt_time{0};
  std::chrono::duration<double, std::micro> total_pfxt_time{0};
  

  const int runs{1};

  // ot
  for (int i = 0; i < runs; i++) {
    ot.report_rebuild(
      k,
      false);
    total_sfxt_time += ot.sfxt_time;
    total_pfxt_time += ot.pfxt_time;
    if (i < runs-1) {
      ot.reset();
    }
  }

  auto ot_avg_sfxt_time = total_sfxt_time/runs/1ms;
  auto ot_avg_pfxt_time = total_pfxt_time/runs/1ms;
  total_sfxt_time = std::chrono::duration<double, std::micro>{0};
  total_pfxt_time = std::chrono::duration<double, std::micro>{0};

  // pathgen
  auto max_cost = golden_costs[k-1];
  auto min_cost = golden_costs.front();
  for (int i = 0; i < runs; i++) {
    pathgen.report_multiq(
      max_cost,
      min_cost,
      k,
      100,
      false,
      true,
      false);
    total_pfxt_time += pathgen.pfxt_time;
    if (i < runs-1) {
      pathgen.reset();
    }
  }
  auto pathgen_avg_pfxt_time = total_pfxt_time/runs/1ms; 
  // get pathgen costs
  auto pathgen_costs = pathgen.get_path_costs_from_cq();
  // calculate max and average error
  float pathgen_max_error = 0.0f;
  float pathgen_avg_error = 0.0f;
  for (size_t i = 0; i < k; i++) {
    float error = std::abs(pathgen_costs[i]-golden_costs[i])/golden_costs[i];
    pathgen_max_error = std::max(pathgen_max_error, error);
    pathgen_avg_error += error;
  }
  pathgen_avg_error /= k;
  std::cout << "pathgen done.\n";

  total_pfxt_time = std::chrono::duration<double, std::micro>{0};

  for (int i = 0; i < runs; i++) {
    cpathgen.report_paths_mlq(
      2.5f, // delta
      k,    // K
      10    // num_queues
    );
    total_pfxt_time += cpathgen.pfxt_time;
    if (i < runs-1) {
      cpathgen.reset();
    }
  }

  // get cpathgen costs
  auto cpathgen_costs = cpathgen.get_path_costs_from_tbb_cv();
 
  // calculate max and average error
  float cpathgen_max_error = 0.0f;
  float cpathgen_avg_error = 0.0f;
  for (size_t i = 0; i < k; i++) {
    float error = std::abs(cpathgen_costs[i]-golden_costs[i])/golden_costs[i];
    cpathgen_max_error = std::max(cpathgen_max_error, error);
    cpathgen_avg_error += error;
  }
  cpathgen_avg_error /= k;
  auto cpathgen_avg_pfxt_time = total_pfxt_time/runs/1ms;
  std::cout << "cpathgen done.\n";


  // dump ot, pathgen, and cpathgen stats to csv
  // header:
  // benchmark, |V|, |E|, ot_sfxt_time, ot_pfxt_time, 
  // pathgen_avg_err, pathgen_max_err, pathgen_avg_pfxt_time (speedup over ot),
  // cpathgen_avg_err, cpathgen_max_err, cpathgen_avg_pfxt_time (speedup over ot)

  // if csv is empty, write header
  std::ofstream ofs("big-table.csv", std::ios::app);
  if (ofs.tellp() == 0) {
    ofs << "benchmark,|V|,|E|,ot_sfxt_time,ot_pfxt_time,"
      << "pathgen_avg_err,pathgen_max_err,pathgen_avg_pfxt_time,"
      << "cpathgen_avg_err,cpathgen_max_err,cpathgen_avg_pfxt_time,"
      << '\n';
  }

  auto vs = ot.num_verts();
  auto es = ot.num_edges();
  ofs << filename << ','
      << vs << ','
      << es << ','
      << std::fixed << std::setprecision(1)
      << ot_avg_sfxt_time << ','
      << ot_avg_pfxt_time << ','
      << pathgen_avg_error*100.0f << ',' << pathgen_max_error*100.0f << ',' 
      << pathgen_avg_pfxt_time << '(' << ot_avg_pfxt_time/pathgen_avg_pfxt_time << "$\\times$),"
      << cpathgen_avg_error*100.0f << ',' << cpathgen_max_error*100.0f << ',' 
      << cpathgen_avg_pfxt_time << '(' << ot_avg_pfxt_time/cpathgen_avg_pfxt_time << "$\\times$)\n";

  return 0;

}
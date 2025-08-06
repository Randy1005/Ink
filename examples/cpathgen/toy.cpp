#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "usage: ./toy [k] [input] [output] [golden]\n";
    return EXIT_FAILURE;
  }

  ink::Ink pathgen;
  size_t k = std::stoi(argv[1]);
  pathgen.read_ops(argv[2], argv[3]);

  pathgen.report_paths_mlq(
    5.0f, // delta
    k,    // K
    10     // num_queues
  );
  std::cout << "Done.\n";


  auto costs = pathgen.get_path_costs_from_tbb_cv();
  std::ofstream ofs(argv[3]); 
  for (const auto& c: costs) {
    ofs << c << '\n';
  }
  ofs.close();


  // read the golden costs
  std::ifstream ifs(argv[4]);
  std::vector<float> golden_costs;
  float cost;
  if (!ifs.is_open()) {
    std::cerr << "Failed to open golden file: " << argv[4] << '\n';
    return EXIT_FAILURE;
  }

  while (ifs >> cost) {
    golden_costs.push_back(cost);
  }
  ifs.close();

  // compare and compute the max/average percentage relative difference
  float max_error = 0.0f;
  float avg_error = 0.0f;
  for (size_t i = 0; i < k; i++) {
    float error = std::abs(costs[i]-golden_costs[i])/golden_costs[i];
    max_error = std::max(max_error, error);
    avg_error += error;
  }

  avg_error /= k;
  std::cout << "Max percentage relative difference: " << max_error*100 << "%\n";
  std::cout << "Average percentage relative difference: " << avg_error*100 << "%\n";

  // print pfxt expansion runtime
  std::cout << "pfxt_expansion_time="
            << pathgen.pfxt_time/1ms << " ms\n";
  return 0;
}

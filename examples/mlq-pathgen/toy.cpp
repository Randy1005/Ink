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
    10,     // num_queues
    true   // ensure_exact 
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

  // print out the k-th cost
  if (costs.size() < k) {
    std::cout << "c-pathgen " << costs.size() << "-th cost: " << costs.back() << '\n';
    std::cout << "golden cost: " << golden_costs[costs.size()-1] << '\n';
  }
  else {
    std::cout << "c-pathgen " << k << "-th cost: " << costs[k-1] << '\n';
    std::cout << "golden cost: " << golden_costs[k-1] << '\n';
  }
  
  return 0;
}

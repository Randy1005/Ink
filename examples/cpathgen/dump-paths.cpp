#include "trace-json.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>

int main(int argc, char* argv[]) {
  if (argc != 5 && argc != 6) {
    std::cerr << "usage: dump-paths [seq|opentimer|pathgen|cpathgen] [k] [input.edges|input.ops] [output.jsonl|-] [num_threads=auto]\n";
    return EXIT_FAILURE;
  }

  const std::string algorithm = argv[1];
  const auto k = static_cast<size_t>(std::stoull(argv[2]));
  const size_t num_threads = (argc == 6) ? static_cast<size_t>(std::stoull(argv[5])) : 0;

  ink::Ink ink;
  if (num_threads > 0) {
    ink.set_num_workers(num_threads);
  }
  ink.read_ops(argv[3], std::string(argv[3]) + "." + algorithm);

  std::vector<ink::Path> paths;
  if (algorithm == "seq" || algorithm == "opentimer") {
    paths = ink.report_rebuild(k, true);
  } else if (algorithm == "pathgen") {
    paths = ink.report_multiq(std::numeric_limits<float>::max(), 0.0f, k, 10, true, true, false);
  } else if (algorithm == "cpathgen") {
    std::optional<size_t> workers = num_threads > 0 ? std::optional<size_t>(num_threads) : std::nullopt;
    paths = ink.report_paths_mlq(0.5f, k, 10, workers, std::nullopt, true);
  } else {
    std::cerr << "unknown algorithm: " << algorithm << "\n";
    return EXIT_FAILURE;
  }

  if (std::string(argv[4]) == "-") {
    cpathgen_trace::write_jsonl(std::cout, paths);
  } else {
    std::ofstream ofs(argv[4]);
    if (!ofs) {
      std::cerr << "failed to open output: " << argv[4] << "\n";
      return EXIT_FAILURE;
    }
    cpathgen_trace::write_jsonl(ofs, paths);
  }
  return EXIT_SUCCESS;
}

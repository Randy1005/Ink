#include <ink/ink.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::vector<size_t> parse_csv_sizes(const std::string& s) {
  std::vector<size_t> out;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, ',')) {
    if (!item.empty()) out.push_back(static_cast<size_t>(std::stoull(item)));
  }
  return out;
}

static std::string basename_no_ext(const std::string& path) {
  fs::path p(path);
  auto stem = p.stem().string();
  return stem.empty() ? p.filename().string() : stem;
}

int main(int argc, char* argv[]) {
  if (argc != 9) {
    std::cerr << "usage: cpathgen-only-sweep [input.edges] [output.csv] [k] [threads_csv] [warmups] [trials] [delta] [num_vecs]\n";
    return EXIT_FAILURE;
  }

  const std::string input = argv[1];
  const fs::path output = argv[2];
  const size_t k = static_cast<size_t>(std::stoull(argv[3]));
  const auto thread_values = parse_csv_sizes(argv[4]);
  const size_t warmups = static_cast<size_t>(std::stoull(argv[5]));
  const size_t trials = static_cast<size_t>(std::stoull(argv[6]));
  const float delta = std::stof(argv[7]);
  const size_t num_vecs = static_cast<size_t>(std::stoull(argv[8]));
  const std::string benchmark = basename_no_ext(input);

  if (!output.parent_path().empty()) {
    fs::create_directories(output.parent_path());
  }
  const bool new_file = !fs::exists(output) || fs::file_size(output) == 0;
  std::ofstream ofs(output, std::ios::app);
  if (!ofs) {
    std::cerr << "failed to open output: " << output << "\n";
    return EXIT_FAILURE;
  }
  if (new_file) {
    ofs << "benchmark,algorithm,k,threads,trial,kernel_ms,sfxt_ms,num_steps,num_paths,delta,num_vecs\n";
  }
  ofs << std::fixed << std::setprecision(6);

  ink::Ink cpathgen;
  cpathgen.read_ops(input, output.string() + ".cpathgen.read_ops.log");

  for (auto threads : thread_values) {
    cpathgen.set_num_workers(threads);
    std::optional<size_t> workers = threads > 0 ? std::optional<size_t>(threads) : std::nullopt;
    for (size_t i = 0; i < warmups; ++i) {
      cpathgen.report_paths_mlq(delta, k, num_vecs, workers, std::nullopt, false);
      cpathgen.reset();
    }
    for (size_t trial = 0; trial < trials; ++trial) {
      cpathgen.report_paths_mlq(delta, k, num_vecs, workers, std::nullopt, false);
      ofs << benchmark << ",cpathgen," << k << ',' << threads << ',' << trial << ','
          << cpathgen.pfxt_time / 1ms << ','
          << cpathgen.sfxt_time / 1ms << ','
          << cpathgen.num_steps << ','
          << (cpathgen.accum_path_cnt_per_step.empty() ? 0 : cpathgen.accum_path_cnt_per_step.back()) << ','
          << delta << ',' << num_vecs << "\n";
      ofs.flush();
      cpathgen.reset();
    }
  }
  return EXIT_SUCCESS;
}

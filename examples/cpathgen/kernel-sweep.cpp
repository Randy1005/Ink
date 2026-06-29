#include <ink/ink.hpp>

#include <algorithm>
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

static void write_header(std::ofstream& ofs) {
  ofs << "benchmark,algorithm,k,threads,trial,kernel_ms,sfxt_ms,num_steps,num_paths,delta,num_vecs\n";
}

int main(int argc, char* argv[]) {
  if (argc != 7 && argc != 9) {
    std::cerr << "usage: kernel-sweep [input.edges] [output.csv] [k_csv] [threads_csv] [warmups] [trials] [delta=0.5] [num_vecs=10]\n";
    return EXIT_FAILURE;
  }

  const std::string input = argv[1];
  const fs::path output = argv[2];
  const auto k_values = parse_csv_sizes(argv[3]);
  const auto thread_values = parse_csv_sizes(argv[4]);
  const size_t warmups = static_cast<size_t>(std::stoull(argv[5]));
  const size_t trials = static_cast<size_t>(std::stoull(argv[6]));
  const float delta = (argc == 9) ? std::stof(argv[7]) : 0.5f;
  const size_t num_vecs = (argc == 9) ? static_cast<size_t>(std::stoull(argv[8])) : 10;
  const std::string benchmark = basename_no_ext(input);

  fs::create_directories(output.parent_path());
  const bool new_file = !fs::exists(output) || fs::file_size(output) == 0;
  std::ofstream ofs(output, std::ios::app);
  if (!ofs) {
    std::cerr << "failed to open output: " << output << "\n";
    return EXIT_FAILURE;
  }
  if (new_file) write_header(ofs);
  ofs << std::fixed << std::setprecision(6);

  ink::Ink opentimer;
  ink::Ink cpathgen;
  opentimer.read_ops(input, (output.string() + ".opentimer.read_ops.log"));
  cpathgen.read_ops(input, (output.string() + ".cpathgen.read_ops.log"));

  for (auto k : k_values) {
    for (size_t i = 0; i < warmups; ++i) {
      opentimer.report_rebuild(k, false);
      opentimer.reset();
    }
    for (size_t trial = 0; trial < trials; ++trial) {
      opentimer.report_rebuild(k, false);
      ofs << benchmark << ",opentimer," << k << ",1," << trial << ','
          << opentimer.pfxt_time / 1ms << ','
          << opentimer.sfxt_time / 1ms << ",,,\n";
      ofs.flush();
      opentimer.reset();
    }

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
            << (cpathgen.accum_path_cnt_per_step.empty() ? 0 : cpathgen.accum_path_cnt_per_step.back())
            << ',' << delta << ',' << num_vecs << "\n";
        ofs.flush();
        cpathgen.reset();
      }
    }
  }

  return EXIT_SUCCESS;
}

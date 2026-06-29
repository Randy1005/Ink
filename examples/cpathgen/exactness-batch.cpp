#include "trace-json.hpp"

#include <ink/ink.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

static void write_prefix_filtered(
  const fs::path& path,
  const std::vector<ink::Path>& paths,
  size_t k,
  bool exclude_zero_cost) {
  fs::create_directories(path.parent_path());
  std::vector<const ink::Path*> selected;
  selected.reserve(std::min(k, paths.size()));
  for (const auto& p : paths) {
    if (exclude_zero_cost && std::abs(p.weight) <= 1.0e-12f) {
      continue;
    }
    selected.push_back(&p);
    if (selected.size() >= k) break;
  }
  std::ofstream ofs(path);
  if (!ofs) {
    throw std::runtime_error("failed to open output " + path.string());
  }
  std::vector<ink::Path> copy;
  copy.reserve(selected.size());
  for (const auto* p : selected) {
    copy.emplace_back(ink::Path(p->weight, p->endpoint));
    for (const auto& point : *p) {
      copy.back().emplace_back(point.vert, point.dist, point.incoming_edge_id, point.incoming_weight_sel);
    }
  }
  cpathgen_trace::write_jsonl(ofs, copy);
}

int main(int argc, char* argv[]) {
  if (argc != 7) {
    std::cerr << "usage: exactness-batch [input.edges] [out_dir] [benchmark] [trace_k] [k_csv] [threads_csv]\n";
    return EXIT_FAILURE;
  }

  const std::string input = argv[1];
  const fs::path out_dir = argv[2];
  const std::string benchmark = argv[3];
  const size_t trace_k = static_cast<size_t>(std::stoull(argv[4]));
  const auto k_values = parse_csv_sizes(argv[5]);
  const auto thread_values = parse_csv_sizes(argv[6]);
  const bool exclude_zero_cost = true;

  fs::create_directories(out_dir);

  ink::Ink golden_ink;
  golden_ink.read_ops(input, (out_dir / (benchmark + ".golden.read_ops.log")).string());
  auto golden_paths = golden_ink.report_rebuild(trace_k, true);
  for (auto k : k_values) {
    write_prefix_filtered(out_dir / ("golden.k" + std::to_string(k) + ".nozero.trace.jsonl"), golden_paths, k, exclude_zero_cost);
  }

  for (auto threads : thread_values) {
    ink::Ink cpg;
    cpg.set_num_workers(threads);
    cpg.read_ops(input, (out_dir / (benchmark + ".cpathgen.t" + std::to_string(threads) + ".read_ops.log")).string());
    auto paths = cpg.report_paths_mlq(0.5f, trace_k, 10, threads, std::nullopt, true);
    for (auto k : k_values) {
      write_prefix_filtered(
        out_dir / ("cpathgen.k" + std::to_string(k) + ".nozero.t" + std::to_string(threads) + ".run0.jsonl"),
        paths,
        k,
        exclude_zero_cost);
    }
  }

  return EXIT_SUCCESS;
}

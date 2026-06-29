#include "trace-json.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "usage: gen-golden-trace [k] [input.edges|input.ops] [output.trace.jsonl]\n";
    return EXIT_FAILURE;
  }

  const auto k = static_cast<size_t>(std::stoull(argv[1]));
  ink::Ink ink_seq;
  ink_seq.read_ops(argv[2], argv[3]);
  auto paths = ink_seq.report_rebuild(k, true);

  std::ofstream ofs(argv[3]);
  if (!ofs) {
    std::cerr << "failed to open output: " << argv[3] << "\n";
    return EXIT_FAILURE;
  }
  cpathgen_trace::write_jsonl(ofs, paths);
  return EXIT_SUCCESS;
}

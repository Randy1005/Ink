#include <ot/timer/timer.hpp>

int main() {
  ot::Timer timer;

  timer.read_celllib("leon2_iccad_Early.lib", ot::MIN)
       .read_celllib("leon2_iccad_Late.lib", ot::MAX)
       .read_verilog("leon2_iccad.v")
       .read_spef("leon2_iccad.spef");

  auto paths = timer.report_timing(10);
  for (size_t i = 0; i < paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

  std::ofstream ofs("../../benchmarks/leon2.edges");
  timer.dump_edge_insertions(ofs);

  return 0;
}

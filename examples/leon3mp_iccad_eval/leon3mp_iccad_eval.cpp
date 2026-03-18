#include <ot/timer/timer.hpp>

int main() {
  ot::Timer timer;

  timer.read_celllib("leon3mp_iccad_Early.lib", ot::MIN)
       .read_celllib("leon3mp_iccad_Late.lib", ot::MAX)
       .read_verilog("leon3mp_iccad.v")
       .read_spef("leon3mp_iccad.spef");

  auto paths = timer.report_timing(10);
  for (size_t i = 0; i < paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

  std::ofstream ofs("../../benchmarks/leon3mp.edges");
  timer.dump_edge_insertions(ofs);

  return 0;
}

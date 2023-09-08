#include <ot/timer/timer.hpp>

int main(int argc, char *argv[]) {

  // create a timer object
  ot::Timer timer;
  
  // Read design
  timer.read_celllib("aes_core_Early.lib", ot::MIN)
       .read_celllib("aes_core_Late.lib", ot::MAX)
       .read_verilog("aes_core.v")
       .read_spef("aes_core.spef");

  // get the top-5 worst critical paths
  auto paths = timer.report_timing(10);

  for(size_t i=0; i<paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

	
	std::ofstream ofs("aes_core.edges");
  timer.dump_edge_insertions(ofs);

  return 0;
}




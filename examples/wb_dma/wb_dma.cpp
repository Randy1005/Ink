#include <ot/timer/timer.hpp>

int main(int argc, char *argv[]) {

  // create a timer object
  ot::Timer timer;
  
  // Read design
  timer.read_celllib("wb_dma_Early.lib", ot::MIN)
       .read_celllib("wb_dma_Late.lib", ot::MAX)
       .read_verilog("wb_dma.v")
       .read_spef("wb_dma.spef");

  // get the top-5 worst critical paths
  auto paths = timer.report_timing(10);

  for(size_t i=0; i<paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

	
	std::ofstream ofs("wb_dma.edges");
  timer.dump_edge_insertions(ofs);

  return 0;
}




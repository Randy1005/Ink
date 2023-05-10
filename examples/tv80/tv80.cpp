#include <ot/timer/timer.hpp>

int main(int argc, char *argv[]) {

  // create a timer object
  ot::Timer timer;
  
  // Read design
  timer.read_celllib("tv80_Early.lib", ot::MIN)
       .read_celllib("tv80_Late.lib", ot::MAX)
       .read_verilog("tv80.v")
       .read_spef("tv80.spef");

  // get the top-5 worst critical paths
  auto paths = timer.report_timing(10);

  for(size_t i=0; i<paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

	
	std::ofstream ofs("tv80.graph");
  timer.dump_graph_ops(ofs, 10, 10000);


  return 0;
}




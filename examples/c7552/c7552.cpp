#include <ot/timer/timer.hpp>

int main(int argc, char *argv[]) {

  // create a timer object
  ot::Timer timer;
  
  // Read design
  timer.read_celllib("c7552_Early.lib", ot::MIN)
       .read_celllib("c7552_Late.lib", ot::MAX)
       .read_verilog("c7552.v")
       .read_spef("c7552.spef")
       .read_sdc("c7552.sdc");

  // get the top-5 worst critical paths
  auto paths = timer.report_timing(10);

  for(size_t i=0; i<paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

	
	std::ofstream ofs("c7552.graph.updates");
  timer.dump_graph_ops(ofs, 1, 10000, true, 1);


  return 0;
}




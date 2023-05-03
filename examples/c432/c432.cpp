#include <ot/timer/timer.hpp>

int main(int argc, char *argv[]) {

  // create a timer object
  ot::Timer timer;
  
  // Read design
  timer.read_celllib("c432_Late.lib", ot::MIN)
       .read_celllib("c432_Late.lib", ot::MAX)
       .read_verilog("c432.v")
       .read_sdc("c432.sdc");

  // get the top-5 worst critical paths
  auto paths = timer.report_timing(5);

  for(size_t i=0; i<paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

	
	std::ofstream ofs("c432.graph");
  timer.dump_graph_ops(ofs, 10, 100);


  return 0;
}




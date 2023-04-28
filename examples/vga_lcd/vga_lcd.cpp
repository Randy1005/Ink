#include <ot/timer/timer.hpp>

int main(int argc, char *argv[]) {

  // create a timer object
  ot::Timer timer;
  
  // Read design
  timer.read_celllib("vga_lcd_Early.lib", ot::MIN)
       .read_celllib("vga_lcd_Early.lib", ot::MAX)
       .read_verilog("vga_lcd.v")
       .read_sdc("vga_lcd.sdc");

  // get the top-5 worst critical paths
  auto paths = timer.report_timing(5);

  for(size_t i=0; i<paths.size(); ++i) {
    std::cout << "----- Critical Path " << i << " -----\n";
    std::cout << paths[i] << '\n';
  }

  // dump the timing graph to dot format for debugging
  //timer.dump_graph_labels(std::cout);
  
	
	std::ofstream ofs("vga_lcd.graph");
  timer.dump_graph_input(ofs);


  return 0;
}




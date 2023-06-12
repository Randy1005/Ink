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

  //std::ofstream ofs("vga_lcd.graph.updates");
  //timer.dump_graph_ops(ofs, 50, 100000, true, 50); 

  std::ofstream ofs("vga_lcd.edges");
  timer.dump_edge_insertions(ofs);
  
  std::ofstream ofs2("vga_lcd.connections");
  timer.dump_connections(ofs2, 10);

  std::ofstream ofs3("vga_lcd.pfxt_srcs");
  


  return 0;
}




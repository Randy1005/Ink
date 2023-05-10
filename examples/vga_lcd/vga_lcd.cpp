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

  std::ofstream ofs("vga_lcd.graph");
  timer.dump_graph_ops(ofs, 50, 100000); 
	


  return 0;
}




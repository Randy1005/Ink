#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "usage: ./a.out [input] [starting-delta] [inc-size] [ending-delta]\n";
    std::exit(EXIT_FAILURE);
  }

  // strip input graph filename from the directory
  std::string filename = argv[1];
  size_t last_slash = filename.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    filename = filename.substr(last_slash+1); 
  }
  // also strip the suffix
  size_t dot_pos = filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    filename = filename.substr(0, dot_pos);
  }

  ink::Ink cpathgen;
  cpathgen.read_ops(argv[1], filename+".cpathgen");

  std::ofstream ofs_delta(filename+".delta-steps-runtime.csv");
  ofs_delta << "delta,num_steps,pfxt_time\n";

  float start_delta = std::stof(argv[2]);
  float inc_size = std::stof(argv[3]);
  float end_delta = std::stof(argv[4]);
  std::vector<std::pair<float, std::ofstream>> ofs_path_per_step;

  for (float d = start_delta; 
       d <= end_delta; 
       d += inc_size) {
    ofs_path_per_step.emplace_back(
      d, std::ofstream(filename+".delta="+std::to_string(d)+".csv")
    );
    
  }

  for (auto& ofs : ofs_path_per_step) {
    ofs.second << "step,accum_path_cnt\n";
  }


  // iterate over different delta values
  const size_t path_cnt = 1000000;
  float d;
  size_t ofs_id;
  for (d = start_delta, ofs_id = 0; 
    d <= end_delta; 
    d += inc_size, ++ofs_id) {
    std::cout << "Running CPathGen with delta=" << d << '\n';
    cpathgen.report_paths_mlq(
      d, 
      path_cnt, 
      10);
    ofs_delta << d << ',' 
              << cpathgen.num_steps << ','
              << cpathgen.pfxt_time/1ms << '\n';
    
    size_t step{0};
    for (const auto& pcnt: cpathgen.accum_path_cnt_per_step) {
      ofs_path_per_step[ofs_id].second 
      << step++ << ',' 
      << pcnt << '\n';
    }
    
    cpathgen.reset();
  }
}
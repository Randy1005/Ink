#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "usage: ./a.out [input] [output] [paths] [num_queues]\n";
    std::exit(EXIT_FAILURE);
  }

  tf::Executor executor;
  ink::Ink ink_seq, ink_par;
  auto k = std::stoi(argv[3]);
  auto qs = std::stoi(argv[4]);

  executor.silent_async([=, &ink_seq] {
    ink_seq.read_ops(argv[1], argv[2]);
  });

  executor.silent_async([=, &ink_par] {
    ink_par.read_ops(argv[1], argv[2]);  
  });

  executor.wait_for_all();

  ink_seq.report_incsfxt(k, false, false);
  ink_par.report_incsfxt(k, false, false);

  auto old_max_dc = ink_seq.get_path_costs().back();

  ink_seq.modify_vertex(127, 0.3f);
  ink_par.modify_vertex(127, 0.3f);


  auto ts = std::thread::hardware_concurrency();
  std::cout << ts << " threads supported.\n";

  ink_seq.report_incsfxt(k, false, false);
  ink_par.report_multiq(old_max_dc, k, qs, false);

  auto costs_seq = ink_seq.get_path_costs();
  auto costs_par = ink_par.get_path_costs_from_cq();

  std::ofstream ofs1("seq-costs");
  std::ofstream ofs2("par-costs");

  for (auto c : costs_seq) {
    ofs1 << c << '\n';
  }
  
  for (auto c : costs_par) {
    ofs2 << c << '\n';
  }

  // output pfxt expansion runtime
  std::cout << "seqen. pfxt expansion time=" << ink_seq.pfxt_expansion_time << '\n';
  std::cout << "paral. pfxt expansion time=" << ink_par.pfxt_expansion_time << '\n';

  return 0;
}

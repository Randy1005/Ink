#include <ink/ink.hpp>


int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "usage: ./a.out [k] [input] [output]\n";
    std::exit(EXIT_FAILURE);
  }

  int k = std::stoi(argv[1]);
  ink::Ink ink_seq;
  ink_seq.read_ops(argv[2], argv[3]);

  
  
  ink_seq.report_incsfxt(k, false, false);
  auto costs = ink_seq.get_path_costs();
  std::ofstream ofs(argv[3]);
  for (const auto& cost: costs) {
    ofs << cost << '\n';
  }

  return 0;
}

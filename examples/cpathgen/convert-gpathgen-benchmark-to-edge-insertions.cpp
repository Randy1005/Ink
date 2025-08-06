#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "usage: ./a.out [input] [output]\n";
    return EXIT_FAILURE;
  }

  ink::Ink ink;
  ink.convert_gpathgen_benchmark_to_edge_insertions(argv[1], argv[2]);

  return 0;
}
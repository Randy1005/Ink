#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "usage: exactness-table [golden.jsonl] [candidate.jsonl] [output.csv]\n";
    std::cerr << "Use experiments/scripts/validate_exactness.py for the canonical validator.\n";
    return EXIT_FAILURE;
  }
  std::cout << "python experiments/scripts/validate_exactness.py --golden "
            << argv[1] << " --candidate " << argv[2]
            << " --output " << argv[3] << "\n";
  return EXIT_SUCCESS;
}

#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
	if (argc != 5) {
		std::cerr 
			<< "usage: ./Ink [graph_ops_file] [output_file] [#paths] [mode]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_ops(argv[1], argv[2]);
	size_t k = std::stoul(argv[3]);
  size_t mode = std::stoul(argv[4]);

	std::cout << "# paths = " << k << '\n';

  ink.report_incsfxt(k, true, false);

  std::uniform_int_distribution<size_t> distr(0, ink.num_verts() - 1);
  size_t vid = distr(ink.rng);

  if (mode == 0) {
    // full cpg mode
    ink.modify_vertex(vid, 0.025f);
    ink.report_rebuild(k, false);
  }
  else {
    // incremental cpg mode
    ink.modify_vertex(vid, 0.025f);
    ink.report_incremental(k, true, false, false);
  }


	return 0;
}



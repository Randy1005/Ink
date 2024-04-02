#include <ink/ink.hpp>

int main(int argc, char* argv[]) {
	if (argc != 5) {
		std::cerr 
			<< "usage: ./Ink [graph_ops_file] [output_file] [#paths] [#iterations]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink i1, i2;
	i1.read_ops(argv[1], argv[2]);
	i2.read_ops(argv[1], argv[2]);
	size_t k = std::stoul(argv[3]);
  size_t num_iters = std::stoul(argv[4]);

	std::cout << "# paths = " << k << '\n';
	std::cout << "# iters = " << num_iters << '\n';

  i1.report_incsfxt(k, true, false);
  i2.report_incsfxt(k, true, false);

  size_t sfxt_update_accum1{0};
  size_t sfxt_update_accum2{0};
  size_t pfxt_expansion_accum1{0};
  size_t pfxt_expansion_accum2{0};
  std::ofstream ofs1("multi-iter-exp-i1.txt");
  std::ofstream ofs2("multi-iter-exp-i2.txt");
  ofs1 << "iteration runtime\n";
  ofs2 << "iteration runtime\n";
  
  std::vector<float> c1;
  std::vector<float> c2;
  std::ofstream ofsf("full-costs"), ofsi("ink-costs");


  
  
  for (size_t it = 0; it < num_iters; it++) {
    std::uniform_int_distribution<size_t> distr(0, i1.num_verts() - 1);
    size_t vid = distr(i1.rng);

    // full cpg mode
    i1.modify_vertex(vid, 0.025f);
    i1.report_rebuild(k, false);
    c1 = i1.get_path_costs();
    sfxt_update_accum1 += i1.sfxt_update_time;
    pfxt_expansion_accum1 += i1.pfxt_expansion_time;
    ofs1 << it + 1 << ' ' << i1.full_time * 1e-6 << '\n';
   
    // incremental cpg mode
    i2.modify_vertex(vid, 0.025f);
    i2.report_incremental(k, true, false, false);
    c2 = i2.get_path_costs();
    sfxt_update_accum2 += i2.sfxt_update_time;
    pfxt_expansion_accum2 += i2.pfxt_expansion_time;
    ofs2 << it + 1 << ' ' << i2.incremental_time * 1e-6 << '\n';
  }

  for (size_t i = 0; i < k; i++) {
    ofsf << c1[i] << '\n';
    ofsi << c2[i] << '\n';
  }

  std::cout << "avg sfxt update time full = " << (sfxt_update_accum1 * 1e-6) / num_iters << '\n';  
  std::cout << "avg pfxt expansion time full = " << (pfxt_expansion_accum1 * 1e-6) / num_iters << '\n';  
  std::cout << "avg sfxt update time inc = " << (sfxt_update_accum2 * 1e-6) / num_iters << '\n';  
  std::cout << "avg pfxt expansion time inc = " << (pfxt_expansion_accum2 * 1e-6) / num_iters << '\n';  


	return 0;
}



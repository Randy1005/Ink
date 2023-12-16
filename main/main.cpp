#include <ink/ink.hpp>
bool equal(float v1, float v2, float eps) {
  return std::fabs(v1 - v2) < eps;
}

bool check_results(
  const std::vector<float>& costs_inc,
  const std::vector<float>& costs_full) {
  for (size_t i = 0; i < costs_full.size(); i++) {
    if (!equal(costs_inc[i], costs_full[i], 0.001f)) {
      std::cout << "mismatch at " << i << "-th path.\n";
      std::cout << "cost_inc=" << costs_inc[i] << '\n';
      std::cout << "cost_full=" << costs_full[i] << '\n';
      return false;
    }
  }
  return true;
}


int main(int argc, char* argv[]) {
	if (argc != 6) {
		std::cerr 
			<< "usage: ./Ink [graph_ops_file] [output_file] [num_paths] [num_iters] [recover_path?]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink_full;
  ink::Ink ink_inc;
	ink_full.read_ops(argv[1], argv[2]);
	ink_inc.read_ops(argv[1], argv[2]);

	size_t num_paths = std::stoul(argv[3]);
	size_t num_iters = std::stoul(argv[4]);
  bool recover_path = std::stoi(argv[5]);
  std::cout << "path count=" << num_paths << '\n';
  std::cout << "num iters=" << num_iters << '\n';


  // dump graph dot
  //ink.dump_graph_dot(std::cout);
 
  // find the edges to insert buffers
  // NOTE: es is a vector of ref_wrappers 
  auto es_full = ink_full.find_chain_edges(); 
  auto es_inc = ink_inc.find_chain_edges(); 
  std::vector<float> costs_full, costs_inc, cache_full, cache_inc;

  std::ofstream ofs_pts_full("out-path-traces-full");
  std::ofstream ofs_pts_inc("out-path-traces-inc");
  std::ofstream ofs_inc("out-inc");
  std::ofstream ofs_full("out-full");
  std::ofstream ofs_rt_full("out-runtime-distr-full");
  std::ofstream ofs_rt_inc("out-runtime-distr-inc");

  // full CPG at the 1st iteration
  auto paths_full = ink_full.report_incsfxt(num_paths, false, recover_path);
  auto paths_inc = ink_inc.report_incsfxt(num_paths, true, recover_path);
  costs_full = ink_full.get_path_costs();
  costs_inc = ink_inc.get_path_costs();
  
  bool check = check_results(costs_inc, costs_full);
  if (!check) {
    std::cout << "full report mismatch!\n";
    std::exit(EXIT_FAILURE);
  }

  cache_full = std::move(costs_full);
  cache_inc = std::move(costs_inc);
  costs_full.clear();
  costs_inc.clear();

  std::deque<std::string> bnames_full; 
  std::deque<std::string> bnames_inc; 

  ofs_rt_full << "iteration runtime\n";
  ofs_rt_inc << "iteration runtime\n";
  float accum_speedup{0.0f};
  float accum_rt_full{0.0f};
  float accum_rt_inc{0.0f};

  for (size_t i = 0; i < num_iters; i++) {
    ofs_rt_full << i+1 << ' ';
    ofs_rt_inc << i+1 << ' ';  
    
    std::cout << "---- iteration " << i << " ----\n";
    if (es_full.size() == 0) {
      std::cout << "used up all available edges at iteration " << i << '\n'; 
      break;
    }

    // grab an edge, create a buffer name
    // and insert buffer
    auto& e_full = es_full.back().get();
    es_full.pop_back();
    auto bname = "BUFF." + e_full.name();
    ink_full.insert_buffer(bname, e_full);
    // store bname, for later buffer removal
    bnames_full.push_back(bname);
    std::cout << "insert " << bname << '\n';   

    auto& e_inc = es_inc.back().get();
    es_inc.pop_back();
    bname = "BUFF." + e_inc.name();
    ink_inc.insert_buffer(bname, e_inc);
    bnames_inc.push_back(bname);

    if (i > 3) {
      // start removing inserted buffers
      auto& bfull = bnames_full.front(); 
      auto& binc = bnames_inc.front();

      ink_full.remove_buffer(bfull);
      ink_inc.remove_buffer(binc);
      std::cout << "remove " << bfull << '\n';   
      bnames_full.pop_front();
      bnames_inc.pop_front();

      // grab another edge and update weight
      auto& efull = es_full.back().get();
      es_full.pop_back();

      es_inc.pop_back();
      
      const auto& fname = efull.from.name;
      const auto& tname = efull.to.name;

      // update edge weight (add arbitrary offset)
      const float offset = 0.03f;
      auto ws = efull.weights;
      for (auto& w : ws) {
        *w += offset;
      }
      ink_full.insert_edge(fname, tname, 
        ws[0], ws[1], ws[2], ws[3],
        ws[4], ws[5], ws[6], ws[7]);
      
      ink_inc.insert_edge(fname, tname, 
        ws[0], ws[1], ws[2], ws[3],
        ws[4], ws[5], ws[6], ws[7]);
      std::cout << "update " << efull.name() << '\n';
    
    }


    // report again
    paths_full = ink_full.report_incsfxt(num_paths, false, recover_path);
    paths_inc = ink_inc.report_incremental_v2(num_paths, true, recover_path);

    costs_full = ink_full.get_path_costs();
    costs_inc = ink_inc.get_path_costs();

    // check results
    cache_full = std::move(costs_full);
    cache_inc = std::move(costs_inc);
    bool check = check_results(cache_inc, cache_full);
    if (!check) {
      std::cout << "mismatch at iteration " << i << "!\n"; 
      break;
    }

    auto rt_full = static_cast<float>(ink_full.pfxt_expansion_time) * 1e-6;
    auto rt_inc = static_cast<float>(ink_inc.pfxt_expansion_time) * 1e-6;

    ofs_rt_full << rt_full << '\n';
    ofs_rt_inc << rt_inc << '\n';
  
    accum_speedup += (static_cast<float>(ink_full.pfxt_expansion_time) / ink_inc.pfxt_expansion_time); 
    accum_rt_full += rt_full;
    accum_rt_inc += rt_inc;    

    costs_full.clear();
    costs_inc.clear();
  }
 
  for (auto c : cache_full) {
    ofs_full << c << '\n';
  }

  for (auto c : cache_inc) {
    ofs_inc << c << '\n';
  }

  if (paths_full.size() != 0 && recover_path) {
    for (size_t i = 0; i < paths_full.size(); i++) {
      ofs_pts_full << "---- Path " << i << " ----\n";
      paths_full[i].dump(ofs_pts_full);
    }

    for (size_t i = 0; i < paths_inc.size(); i++) {
      ofs_pts_inc << "---- Path " << i << " ----\n";
      paths_inc[i].dump(ofs_pts_inc);
    }
  }

  std::cout << std::setprecision(3) << "Avg. speedup = " << accum_speedup / num_iters << '\n';
  std::cout << std::setprecision(4) << "Avg. full CPG runtime  = " << accum_rt_full / num_iters << '\n';
  std::cout << std::setprecision(4) << "Avg. Ink runtime  = " << accum_rt_inc / num_iters << '\n';


 	return 0;
}



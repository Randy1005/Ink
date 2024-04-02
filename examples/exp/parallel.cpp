#include <ink/ink.hpp>
#include <filesystem>

const	float eps = 0.001f;
bool float_eq(const float f1, const float f2) {
	return std::fabs(f1 - f2) < eps;
}


int main(int argc, char* argv[]) {
  if (argc != 12) {
    std::cerr << "usage: ./a.out [input] [output] [paths] [num_queues] [num_workers] [bulk_size] [enable_node_redistribution?] [policy=0|1] [overgrow_scalar] [runs] [os=0|1]\n";
    std::exit(EXIT_FAILURE);
  }

  tf::Executor executor;
  ink::Ink ink_seq, ink_par;
  auto k = std::stoi(argv[3]);
  auto qs = std::stoi(argv[4]);
  auto num_workers = std::stoi(argv[5]);
  auto bulk_size = std::stoi(argv[6]);
  auto enable_node_redistr = std::stoi(argv[7]);
  ink_par.policy = static_cast<ink::PartitionPolicy>(std::stoi(argv[8]));
  ink_par.overgrow_scalar = std::stof(argv[9]);
  auto runs = std::stoul(argv[10]);
  auto o_str = std::stoi(argv[11]);
  
  // set num workers for ink_par
  ink_par.set_num_workers(num_workers);

  // set bulk size
  ink_par.set_dequeue_bulk_size(bulk_size);

  executor.silent_async([=, &ink_seq] {
    ink_seq.read_ops(argv[1], argv[2]);
  });

  executor.silent_async([=, &ink_par] {
    ink_par.read_ops(argv[1], argv[2]);  
  });

  executor.wait_for_all();

  size_t accum_exp_time_seq{0}, accum_exp_time_par{0};
  std::vector<float> costs_seq, costs_par;
  float max_accuracy{0.0f}, min_accuracy{100.0f}, accum_accuracy{0.0f};
  
  auto ins_pts_seq = ink_seq.find_chain_edges(); 
  auto ins_pts_par = ink_par.find_chain_edges(); 
  std::deque<std::string> bnames_seq; 
  std::deque<std::string> bnames_par; 
  
  for (size_t i = 0; i < runs; i++) {
    std::cout << "==== run " << i << " ====\n";
    ink_seq.report_incsfxt(k, false, false);
    costs_seq = std::move(ink_seq.get_path_costs());
    auto old_max_dc = costs_seq.back();
    auto old_min_dc = costs_seq.front();

    // pick up a vertex and modify
    const size_t range_min = 0;
    const size_t range_max = ink_seq.num_verts() - 1;
    std::random_device rng_dev;
    std::mt19937 gen{rng_dev()};
    std::uniform_int_distribution<size_t> distr(range_min, range_max);
    auto vid = distr(gen);
    ink_seq.modify_vertex(vid, -0.3f);
    ink_par.modify_vertex(vid, -0.3f);
    std::cout << "modify weights of vid=" << vid << '\n';

    // grab an edge, create a buffer name
    // and insert buffer
    auto& e_s = ins_pts_seq.back().get();
    ins_pts_seq.pop_back();
    auto& e_p = ins_pts_par.back().get();
    ins_pts_par.pop_back();
    auto bname = "BUFF." + e_s.name();
    ink_seq.insert_buffer(bname, e_s);
    ink_par.insert_buffer(bname, e_p);
    // store bname, for later buffer removal
    bnames_seq.push_back(bname);
    bnames_par.push_back(bname);
    std::cout << "insert " << bname << '\n';   

    //auto& r = bnames.front(); 
    //if (i > 3 && r != bname) {
    //  // start removing inserted buffers
    //  ink_seq.remove_buffer(r);
    //  ink_par.remove_buffer(r);
    //  std::cout << "remove " << r << '\n';   
    //  bnames.pop_front();
    //  bnames.pop_front();
    //}

    ink_seq.report_incsfxt(k, false, false);
    ink_par.report_multiq(old_max_dc, old_min_dc, k, qs, false, enable_node_redistr, false);
    
    // accumulate pfxt expansion time
    accum_exp_time_seq += ink_seq.pfxt_expansion_time;
    accum_exp_time_par += ink_par.pfxt_expansion_time;
  
    costs_seq = std::move(ink_seq.get_path_costs());
    costs_par = std::move(ink_par.get_path_costs_from_cq());
 
    // measure accuracy
    size_t j{0};
    for (; j < costs_seq.size(); j++) {
      if (!float_eq(costs_seq[j], costs_par[j])) {
        std::cout << "mismatch at [" << j << "].\n";
        std::cout << "seq-cost=" << costs_seq[j] << '\n';
        std::cout << "par-cost=" << costs_par[j] << '\n';
        std::cout << "diff=" << std::fabs(costs_seq[j] - costs_par[j]) << '\n';
        break;
      }
    }
    auto accuracy = static_cast<float>(j) * 100.0f / costs_seq.size();
    //std::cout << "accuracy of run " << i << " is "
    //          << accuracy << " %\n";
    max_accuracy = std::max(max_accuracy, accuracy);
    min_accuracy = std::min(min_accuracy, accuracy);
    accum_accuracy += accuracy;
  }

  // output pfxt expansion runtime
  auto seq_rt = static_cast<float>(accum_exp_time_seq) / runs;
  auto par_rt = static_cast<float>(accum_exp_time_par) / runs;
  
  auto out_stats = [&](std::ostream& os) -> void {
    os << "avg_seq_rt " << seq_rt * 1e-6 << '\n'; 
    os << "avg_par_rt " << par_rt * 1e-6 << '\n'; 
    os << "speedup " << seq_rt / par_rt << '\n'; 
    os << "avg_acc " << accum_accuracy / runs << '\n';
    os << "min_acc " << min_accuracy << '\n';
    os << "max_acc " << max_accuracy << '\n';
  };

  // report.csv row format:
  // benchmark |V| |E| #paths #Q #W bulk_sz node_redistr? part_policy  avg_seq_rt avg_par_rt
  // speedup min_acc max_acc avg_acc
  auto write2rpt = [&](std::ostream& os) -> void {
    os << argv[1] << ',';
    os << ink_par.num_verts() << ',';
    os << ink_par.num_edges() << ',';
    os << k << ',';
    os << qs << ',';
    os << num_workers << ',';
    os << bulk_size << ',';
    os << enable_node_redistr << ','; 
    if (ink_par.policy == ink::PartitionPolicy::EQUAL) {
      os << "EQUAL,";
    }
    else if (ink_par.policy == ink::PartitionPolicy::GEOMETRIC) {
      os << "GEO,";
    }
    os << seq_rt * 1e-6 << ',';    
    os << par_rt * 1e-6 << ',';
    os << seq_rt / par_rt << ',';
    os << min_accuracy << ',';
    os << max_accuracy << ',';
    os << accum_accuracy / runs << '\n';
  };

  if (o_str == 0) {
    // output to file
    std::ofstream ofs("stats.out");
    //out_stats(ofs);
  }
  else if (o_str == 1) {
    // output to std::cout
    //out_stats(std::cout);
  } 
  
  std::ofstream ofs_rpt;
  if (!std::filesystem::exists("report.csv")) {
    std::cout << "first time creating report.\n";
    ofs_rpt.open("report.csv", std::ios::app);
    ofs_rpt << "benchmark,|V|,|E|,paths,#Q,#W,bulk_size,node_redistr?,partition_policy,seq_runtime,par_runtime,speedup,min_acc,max_acc,avg_acc\n";
    write2rpt(ofs_rpt);
  }
  else {
    ofs_rpt.open("report.csv", std::ios::app);
    write2rpt(ofs_rpt);
  }

  return 0;
}

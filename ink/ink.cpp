#include "ink.hpp"

namespace ink {

// ------------------------
// Vertex Implementations
// ------------------------
bool Vert::is_src() const {
	// no fanins
	if (num_fanins() == 0) {
		return true;
	}

	return false;
}

bool Vert::is_dst() const {
	// no fanouts
	if (num_fanouts() == 0) {
		return true;
	}

	return false;
}

void Vert::insert_fanin(Edge& e) {
	e.fanin_satellite = fanin.insert(fanin.end(), &e);
}

void Vert::insert_fanout(Edge& e) {
	e.fanout_satellite = fanout.insert(fanout.end(), &e);
}

void Vert::remove_fanin(Edge& e) {
	assert(e.fanin_satellite);
	assert(&e.to == this);
	fanin.erase(*(e.fanin_satellite));

	// invalidate satellite iterator
	e.fanin_satellite.reset();
}

void Vert::remove_fanout(Edge& e) {
	assert(e.fanout_satellite);
	assert(&e.from == this);
	fanout.erase(*(e.fanout_satellite));

	// invalidate satellite iterator
	e.fanout_satellite.reset();
}

// ------------------------
// Edge Implementations
// ------------------------
std::string Edge::name() const {
	return from.name + "->" + to.name;
}

inline size_t Edge::min_valid_weight() const {
	float v = std::numeric_limits<float>::max();
	size_t min_idx = NUM_WEIGHTS;
	for (size_t i = 0; i < NUM_WEIGHTS; i++) {
		if (weights[i] && *weights[i] < v) {
			min_idx = i;
			v = *weights[i];
		}
	}
	return min_idx;
}


// ------------------------
// Ink Implementations
// ------------------------
void Ink::read_ops(const std::string& in, const std::string& out) {
	std::ifstream ifs;
	ifs.open(in);
	if (!ifs) {
		throw std::runtime_error("Failed to open file: " + in);
	}

	std::ofstream ofs(out);

	_read_ops(ifs, ofs);
}


Vert& Ink::insert_vertex(const std::string& name) {
	// NOTE: 
	// 1. (vertex non-existent): simply insert
	// if a valid id can be popped from the free list, assign
	// it to this new vertex
	// if no free id is available, increment the size of storage
	// 2. (vertex exists): do nothing

	// check if a vertex with this name exists
	auto itr = _name2v.find(name);

	if (itr == _name2v.end()) {
		// store in name to object map
		auto id = _idxgen_vert.get();

		// NOTE: Vert is a rvalue, compiler moves it by default
		auto [iter, success] = _name2v.emplace(name, Vert{name, id});
	
		// if the index generated goes out of range, resize _vptrs 
		if (id + 1 > _vptrs.size()) {
			_vptrs.resize(id + 1, nullptr);
			
		}

		_vptrs[id] = &(iter->second); 
		return iter->second;	
	}
	else {
		return itr->second;
	}
	
	
}

void Ink::remove_vertex(const std::string& name) {
	auto itr = _name2v.find(name);

	if (itr == _name2v.end()) {
		// vertex non-existent, nothing to do
		return;
	}
	

	// iterate through this vertex's
	// fanin and fanouts and remove them
	auto& v = itr->second;

	for (auto e : v.fanin) {
		// record each of e's fanin vertex
		// they need to be re-relaxed during 
		// incrmental report
		if (!_vptrs[e->from.id]->is_in_update_list && _global_sfxt) {
			_to_update.emplace_back(e->from.id);
			_vptrs[e->from.id]->is_in_update_list = true;
		}
		
		
		// remove edge pointer mapping and recycle free id
		_eptrs[e->id] = nullptr;
		_idxgen_edge.recycle(e->id);
		e->from.remove_fanout(*e);
		assert(e->satellite);
		_edges.erase(*e->satellite);
	}
	for (auto e : v.fanout) {
		// record each of e's fanout vertex
		// they need to be re-relaxed during 
		// incrmental report
		if (!_vptrs[e->to.id]->is_in_update_list && _global_sfxt) {
			_to_update.emplace_back(e->to.id);
			_vptrs[e->to.id]->is_in_update_list = true;
		}

		_eptrs[e->id] = nullptr;
		_idxgen_edge.recycle(e->id);
		e->to.remove_fanin(*e);
		assert(e->satellite);
		_edges.erase(*e->satellite);
	}


	// remove vertex pointer mapping and recycle free id
	_vptrs[v.id] = nullptr;
	_idxgen_vert.recycle(v.id);
	_name2v.erase(itr);

}

Edge& Ink::insert_edge(
	const std::string& from,
	const std::string& to,
	const std::optional<float> w0, 
	const std::optional<float> w1, 
	const std::optional<float> w2, 
	const std::optional<float> w3,
	const std::optional<float> w4, 
	const std::optional<float> w5, 
	const std::optional<float> w6, 
	const std::optional<float> w7) {

	std::array<std::optional<float>, NUM_WEIGHTS> ws = {
		w0, w1, w2, w3, w4, w5, w6, w7
	};
		
	Vert& v_from = insert_vertex(from);
	Vert& v_to = insert_vertex(to);

	const std::string ename = from + "->" + to;
	auto itr = _name2eit.find(ename);

	if (itr != _name2eit.end()) {
		// edge exists
		auto& e = *itr->second;

		// update weights
		for (size_t i = 0; i < e.weights.size(); i++) {
			e.weights[i] = ws[i];
		}
		
		// record from vertex
		if (_global_sfxt) {
			auto v = e.from.id;
			if (!_vptrs[v]->is_in_update_list) {
				_to_update.emplace_back(v);
				_vptrs[v]->is_in_update_list = true;
			}

			e.modified = true;
		}

		// record affected pfxt nodes
		for (auto n : e.dep_nodes) {
			_affected_pfxtnodes.push_back(std::move(n));
		}
		e.dep_nodes.clear();

		return e;
	}
	else {
		// edge doesn't exist
		auto id = _idxgen_edge.get();

		auto& e = _edges.emplace_front(v_from, v_to, id, std::move(ws));
		
		// cache the edge's satellite iterator
		// in the owner storage
		e.satellite = _edges.begin();
		
		// resize _eptr if the generated index goes out of range
		if (id + 1 > _eptrs.size()) {
			_eptrs.resize(id + 1, nullptr);
		}

		// store raw pointer to this edge object
		_eptrs[id] = &e;

		// update fanin, fanout
		// cache fanin, fanout satellite iterators
		// (for edge removal)
		e.from.insert_fanout(e);
		e.to.insert_fanin(e);

		// update the edge name to iterator mapping
		_name2eit.emplace(ename, _edges.begin());
	
		// record from vertex 
		if (_global_sfxt) {
			auto v = e.from.id;
			if (!_vptrs[v]->is_in_update_list) {
				_to_update.emplace_back(v);
				_vptrs[v]->is_in_update_list = true;
			}

			e.modified = true;
		}

		return e; 
	}
}

Edge& Ink::get_edge(const std::string& from, const std::string& to) {
	const std::string ename = from + "->" + to;
	auto itr = _name2eit.find(ename);
	return *itr->second;
}

void Ink::remove_edge(const std::string& from, const std::string& to) {
  
	const std::string ename = from + "->" + to;
	auto itr = _name2eit.find(ename);
	
	if (itr == _name2eit.end()) {
		// edge non existent, nothing to do
		return;
	}
	
	_remove_edge(*itr->second);
	
	// remove name to edge iterator mapping
	_name2eit.erase(itr);
}


std::vector<Path> Ink::report(size_t K) {
	if (K == 0) {
		return {};
	}
	
	// TODO: K = 1
	if (K == 1) {
	
	}

	// incremental: clear endpoint storage
	_endpoints.clear();

	// scan and add the out-degree=0 vertices to the endpoint vector
	for (const auto v : _vptrs) {
		if (v != nullptr && v->is_dst()) {
			_endpoints.emplace_back(*v, 0.0f);	
		}
	}

	PathHeap heap;
	_taskflow.transform_reduce(_endpoints.begin(), _endpoints.end(), heap,
		[&] (PathHeap l, PathHeap r) mutable {
			l.merge_and_fit(std::move(r), K);
			return l;
		},
		[&] (Point& ept) {
			PathHeap heap;
			_spur(ept, K, heap);
			return heap;
		}
	);

	_executor.run(_taskflow).wait();
	_taskflow.clear();

	_clear_update_list();		
	return heap.extract();
}

std::vector<Path> Ink::report_incsfxt(
	size_t K, 
	bool save_pfxt_nodes,
	bool recover_paths) {
	if (K == 0) {
		return {};
	}
	
	// TODO: K = 1
	if (K == 1) {
	
	}

	// incremental: clear endpoint storage
	_endpoints.clear();

	// scan and add the out-degree=0 vertices to the endpoint vector
	for (const auto v : _vptrs) {
		if (v != nullptr && v->is_dst()) {
			_endpoints.emplace_back(*v, 0.0f);	
		}
	}

	PathHeap heap;
	_sfxt_cache();
	auto& sfxt = *_global_sfxt;
	auto pfxt = _pfxt_cache(sfxt);

	auto beg = std::chrono::steady_clock::now();
	_spur_incsfxt(K, heap, pfxt, save_pfxt_nodes, recover_paths);	
	auto end = std::chrono::steady_clock::now();
	_elapsed_time_spur = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
	
	// report complete, clear the update list
	_clear_update_list();		
	
	return _extract_paths(_all_paths);
}

std::vector<Path> Ink::report_rebuild(size_t K) {
	if (K == 0) {
		return {};
	}
	
	// TODO: K = 1
	if (K == 1) {
	
	}

	// incremental: clear endpoint storage
	_endpoints.clear();

	// scan and add the out-degree=0 vertices to the endpoint vector
	for (const auto v : _vptrs) {
		if (v != nullptr && v->is_dst()) {
			_endpoints.emplace_back(*v, 0.0f);	
		}
	}

	PathHeap heap;
	auto beg = std::chrono::steady_clock::now();
	_spur_rebuild(K, heap);	
	auto end = std::chrono::steady_clock::now();
	auto elapsed = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
	_elapsed_time_spur = elapsed;
	
	// report complete, clear the update list
	_clear_update_list();		

	return heap.extract();
}

std::vector<Path> Ink::report_incremental(
	size_t K,
	bool save_pfxt_nodes,
	bool use_leaders,
	bool recover_paths) {
	if (K == 0) {
		return {};
	}
	
	// TODO: K = 1
	if (K == 1) {
	
	}

	// incremental: clear endpoint storage
	_endpoints.clear();

	// scan and add the out-degree=0 vertices to the endpoint vector
	for (const auto v : _vptrs) {
		if (v != nullptr && v->is_dst()) {
			_endpoints.emplace_back(*v, 0.0f);	
		}
	}

	_sfxt_cache();
	auto& sfxt = *_global_sfxt;
	auto pfxt = _pfxt_cache(sfxt);
	pfxt.nodes = std::move(_pfxt_nodes);
	pfxt.paths = std::move(_pfxt_paths);

	auto beg = std::chrono::steady_clock::now();
	_spur_incremental(K, pfxt, save_pfxt_nodes, use_leaders, recover_paths);	
	auto end = std::chrono::steady_clock::now();
	_elapsed_time_spur = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();

	// report complete, clear the update list
	_clear_update_list();	
	_num_affected_nodes = _affected_pfxtnodes.size();
	_affected_pfxtnodes.clear();

	return _extract_paths(_all_paths);
}


void Ink::dump(std::ostream& os) const {
	os << num_verts() << " Vertices:\n";
	os << "------------------\n";
	for (const auto& [name, v] : _name2v) {
		os << "name=" << v.name 
			 << ", id=" << v.id << '\n'; 
		os << "... " << v.fanin.size() << " fanins=";
		for (const auto& e : v.fanin) {
			os << e->from.name << ' ';
		}
		os << '\n';
	
		os << "... " << v.fanout.size() << " fanouts=";
		for (const auto& e : v.fanout) {
			os << e->to.name << ' ';
		}
		os << '\n';
	}

	os << "------------------\n";
	os << " Vert Ptrs:\n";
	os << "------------------\n";
	for (const auto& p : _vptrs) {
		if (p != nullptr) {
			os << "ptr to " << p->name << '\n';
		}
		else {
			os << "ptr to null\n";
		}
	}

	os << "------------------\n";
	os << num_edges() << " Edges:\n";
	os << "------------------\n";
	for (const auto& e : _edges) {
		os << "id=" << e.id
			 << " ... name= " << e.from.name 
			 << "->" << e.to.name
			 << " ... weights= ";
		for (const auto& w : e.weights) {
			if (w.has_value()) {
				os << w.value() << ' ';
			}
			else {
				os << "n/a ";
			}
		}
		os << '\n';
	}

	
	os << "------------------\n";
	os << " Edges Ptrs:\n";
	os << "------------------\n";
	for (const auto& e : _eptrs) {
		if (e != nullptr) {
			os << "ptr to " << e->from.name << "->" << e->to.name << '\n';
		}
		else {
			os << "ptr to null\n";
		}
	}
}

void Ink::dump_pfxt_srcs(std::ostream& os) const {
	for (auto src : _pfxt_srcs) {
		for (auto c : src->children) {
			if (c->edge) {
				os << "insert_edge " 
					 << c->edge->from.name
					 << ' '
					 << c->edge->to.name
					 << ' ';

				for (size_t i = 0; i < NUM_WEIGHTS; i++) {
					if (c->edge->weights[i]) {
						os << *c->edge->weights[i] << ' ';
					}
					else {
						os << "n/a ";
					}
				}
				os << '\n';
			}
		}
	
	}
}

void Ink::dump_pfxt_nodes(std::ostream& os) const {
	// dump pfxt node 
	// edge id, weight sel
	for (auto& n : _pfxt_nodes) {
		if (n->edge) {
			os << n->edge->id << '\n';
		}		
	}	

}


void Ink::dump_profile(std::ostream& os, bool reset) {
	os << "======== Profile ========\n";
	os << "identify leader time: " << _elapsed_time_idl << " ns ("
		 << _elapsed_time_idl / 1000000000.0f << " sec)\n";
	//os << "prop tree time: " << _elapsed_prop << " ns ("
	//	 << _elapsed_prop / 1000000000.0f << " sec)\n";
	os << "spur loop: " << _elapsed_time_sploop << " ns ("
		 << _elapsed_time_sploop / 1000000000.0f << " sec)\n";
	os << "search space expansions: " << _sses << '\n';
	os << "subtree propagations: " << _props << '\n';
	os << "num of affected nodes: " << _num_affected_nodes << '\n';
	os << "num of leaders: " << _leader_nodes.size() << '\n';
	if (reset) {
		_elapsed_time_spur = 0;
		_elapsed_time_spur2 = 0;
		_elapsed_time_sploop = 0;
		_elapsed_time_idl = 0;
		_elapsed_prop = 0;
		_elapsed_time_tr = 0;
		_elapsed_final_path_sort = 0;
		_elapsed_init = 0;
		_elapsed_sse = 0;
		_sses = 0;
		_props = 0;
		_elapsed_pop = 0;
		_max_precomp_nodes = 0;
		_max_nodes = 0;
		_sort_cnt = 0;
		_skip_heaps = 0;
		_leader_cnt = 0;
	}
	os << "=========================\n\n";
}


void Ink::modify_random_vertex() {
	//std::uniform_int_distribution<uint32_t> uint_dist(0, num_verts());

	//const size_t vid = uint_dist(_rng);
	//auto vptr = _vptrs[vid];
	//
	//std::array<std::optional<float>, NUM_WEIGHTS> ws;

	auto& v = insert_vertex("inst_83497:RN");
	
	for (auto e : v.fanout) {
		for (size_t i = 0; i < NUM_WEIGHTS; i++) {
			*e->weights[i] -= 0.025f;
		}
	}

	for (auto e : v.fanin) {
		for (size_t i = 0; i < NUM_WEIGHTS; i++) {
			*e->weights[i] -= 0.025f;
		}
	}




}

float Ink::vec_diff(
	const std::vector<Path>& ps1, 
	const std::vector<Path>& ps2,
	std::vector<float>& diff) {
	
	std::vector<float> ws1;
	for (auto& p : ps1) {
		ws1.emplace_back(p.weight);
	}

	std::vector<float> ws2;
	for (auto& p : ps2) {
		ws2.emplace_back(p.weight);
	}

	std::set_difference(
		ws1.begin(), 
		ws1.end(),
		ws2.begin(),
		ws2.end(),
		std::inserter(diff, diff.begin()));


	const float diff_size = static_cast<float>(diff.size());
	return diff_size / ws1.size();
}


std::vector<float> Ink::get_path_costs() {
	auto costs = std::move(_all_path_costs);
	_all_path_costs.clear();
	return costs;
}

void Ink::_read_ops(std::istream& is, std::ostream& os) {
	std::string buf;
	while (true) {
		is >> buf;
		if (is.eof()) {
			break;
		}

		if (buf == "#") {
			std::getline(is, buf);
			continue;
		}
		
		if (buf == "report_incsfxt") {
			is >> buf;
			size_t num_paths = std::stoul(buf);

			// enable save_pfxt_nodes or not
			is >> buf;
			bool save_pfxt_nodes = std::stoi(buf);

			auto paths = report_incsfxt(num_paths, save_pfxt_nodes);
			os << paths.size() << '\n';
			for (const auto& p : paths) {
				os << p.weight << ' ';
			}
			os << '\n';
		}

			
		if (buf == "report_incremental") {
			is >> buf;
			size_t num_paths = std::stoul(buf);

			// enable save_pfxt_nodes?
			is >> buf;
			bool save_pfxt_nodes = std::stoi(buf);

			// enable use_leaders or not
			is >> buf;
			bool use_leaders = std::stoi(buf);
			
			auto paths = report_incremental(num_paths, save_pfxt_nodes, use_leaders);
			os << paths.size() << '\n';
			for (const auto& p : paths) {
				os << p.weight << ' ';
			}
			os << '\n';
		}



		if (buf == "insert_edge") {
			std::array<std::optional<float>, 8> ws;
			is >> buf;
			std::string from(buf);
			is >> buf;
			std::string to(buf);
			
			for (size_t i = 0; i < NUM_WEIGHTS; i++) {
				is >> buf;
				if (buf == "n/a") {
					ws[i] = std::nullopt;
				}
				else {
					ws[i] = stof(buf);
				}
			}

			insert_edge(from, to,
				ws[0], ws[1], ws[2], ws[3],
				ws[4], ws[5], ws[6], ws[7]);
		}


	}
	
	std::cout << "finished reading " << num_edges() << " edges.\n";	
	std::cout << "finished reading " << num_verts() << " vertices.\n";
	
}

Edge& Ink::_insert_edge(
	Vert& from, 
	Vert& to, 
	std::array<std::optional<float>, 8>&& ws) {
	
	auto id = _idxgen_edge.get();
	auto& e = _edges.emplace_front(from, to, id, std::move(ws));
	
	// cache the edge's satellite iterator
	// in the owner storage
	e.satellite = _edges.begin();
	
	// resize _eptr if the generated index goes out of range
	if (id + 1 > _eptrs.size()) {
		_eptrs.resize(id + 1, nullptr);
	}

	// store pointer to this edge object
	_eptrs[id] = &e;

	// update fanin, fanout
	// cache fanin, fanout satellite iterators
	// (for edge removal)
	e.from.insert_fanout(e);
	e.to.insert_fanin(e);
	return e; 
}


void Ink::_remove_edge(Edge& e) {
	// update fanout of v_from, fanin of v_to
	e.from.remove_fanout(e);
	e.to.remove_fanin(e);


	// record from
	if (_global_sfxt) {
		auto v = e.from.id;
		if (!_vptrs[v]->is_in_update_list) {
			_to_update.emplace_back(v);
			_vptrs[v]->is_in_update_list = true;
		}
	}


	// remove pointer mapping and recycle free id
	_eptrs[e.id] = nullptr;
	_idxgen_edge.recycle(e.id);
	
	// remove this edge from the owner storage
	assert(e.satellite);
	_edges.erase(*e.satellite);


}

void Ink::_topologize(Sfxt& sfxt, size_t v) const {	
	// set visited to true
	sfxt.visited[v] = true;
	auto& vert = _vptrs[v];
	// base case: stop at path source
	if (!vert->is_src()) {
		for (auto& e : vert->fanin) {
			if (!sfxt.visited[e->from.id]) {
				_topologize(sfxt, e->from.id);
			}			
		}
	}
	
	sfxt.topo_order.push_back(v);
}

void Ink::_build_sfxt(Sfxt& sfxt) const {
	// NOTE: super source and target are implicitly generated
	assert(sfxt.topo_order.empty());
	// generate topological order of vertices
	if (sfxt.S > sfxt.T) {
		_topologize(sfxt, sfxt.T);
	}
	else {
		// topologizing for a global sfxt
		for (const auto& ept : _endpoints) {
			_topologize(sfxt, ept.vert.id);
		}
	}

	assert(!sfxt.topo_order.empty());

	// visit the topological order in reverse
	for (auto itr = sfxt.topo_order.rbegin(); 
		itr != sfxt.topo_order.rend(); 
		++itr) {
		auto v = *itr;
		auto v_ptr = _vptrs[v]; 
		assert(v_ptr != nullptr);	

		if (v_ptr->is_src()) {
			sfxt.srcs.try_emplace(v, std::nullopt);
			continue;
		}
		

		for (auto e : v_ptr->fanin) {
			auto w_sel = e->min_valid_weight();

			// relaxation
			if (w_sel != NUM_WEIGHTS) {
				auto d = *e->weights[w_sel];
				sfxt.relax(e->from.id, v, _encode_edge(*e, w_sel), d);	
			}
		}

	}	

}

void Ink::_sfxt_cache() {
	
	// now id 0 and 1 are reserved for super source and super target
	size_t S = 0, T = 1;
	
	// clear and resize the table
	// which we use to check link updates
	belongs_to_sfxt.clear();
	belongs_to_sfxt.resize(num_edges());
	
	if (!_global_sfxt) {
		// it's our first time constructing a global sfxt
		_global_sfxt = Sfxt(S, T, _vptrs.size());
		auto& sfxt = *_global_sfxt;
		assert(!sfxt.dists[T]);
		sfxt.dists[T] = 0.0f;
		
		_build_sfxt(sfxt);
		
		// relax from destinations to super target
		for (auto& p : _endpoints) {
			sfxt.relax(p.vert.id, T, std::nullopt, 0.0f);
		}

		// relax from super source to sources
		for (auto& [src, w] : sfxt.srcs) {
			w = 0.0f;
			sfxt.relax(S, src, std::nullopt, *w);
		}
	}
	else {
		// we have an existing suffix tree
		auto& sfxt = *_global_sfxt;
		auto sz = _vptrs.size();
		sfxt.dists.resize(sz);
		sfxt.successors.resize(sz);
		sfxt.links.resize(sz);
		sfxt.srcs.clear();

		// update sources
		for (const auto v : _vptrs) {
			if (v != nullptr && v->is_src()) {
				sfxt.srcs.try_emplace(v->id, std::nullopt);
			}
		}

				
		// identify the affected region of vertices
		// generate the topological order with DFS
		std::vector<bool> checked(_vptrs.size(), false);
		std::deque<size_t> tpg;
		for (const auto& v : _to_update) {
			if (!checked[v]) {
				_dfs(v, tpg, checked);
			}
		}
		
		// iterate through the topological order and re-relax
		for (const auto& v : tpg) {
			auto vptr = _vptrs[v];
			sfxt.dists[v].reset();

			if (vptr->is_dst()) {
				sfxt.links[v].reset();
				sfxt.successors[v] = sfxt.T;
				continue;
			}


			// cache the current sfxt link of v, 
			// so we can check if the link got updated
			auto old_link = sfxt.links[v];

			// for each of v's fanout we redo relaxation
			for (auto e : vptr->fanout) {
				auto t = e->to.id;
				auto w_sel = e->min_valid_weight();
				if (w_sel != NUM_WEIGHTS) {
			
					auto d = *e->weights[w_sel];
					sfxt.relax(v, t, _encode_edge(*e, w_sel), d);
				}
			}	
		
			auto [eptr_new, w_sel_new] = _decode_edge(*sfxt.links[v]);
			auto [eptr_old, w_sel_old] = _decode_edge(*old_link);
			// compare the cached link to the current link
			if (eptr_new && eptr_old &&
					sfxt.links[v] != old_link) {
				// this means another edge took over and became the successor
				// or another weight is selected for the same edge
				belongs_to_sfxt[eptr_new->id][w_sel_new] = true;
				belongs_to_sfxt[eptr_old->id][w_sel_old] = false;
			}

		}	
		
				
		// relax from destinations to super target
		for (auto& p : _endpoints) {
			sfxt.relax(p.vert.id, T, std::nullopt, 0.0f);
		}

		// relax from super source to sources
		for (auto& [src, w] : sfxt.srcs) {
			w = 0.0f;
			sfxt.relax(S, src, std::nullopt, *w);
		}
	}
}

void Ink::_sfxt_rebuild() {
	// now id 0 and 1 are reserved for super source and super target
	size_t S = 0, T = 1;

	_global_sfxt = Sfxt(S, T, _vptrs.size());
	auto& sfxt = *_global_sfxt;
	assert(!sfxt.dists[T]);
	sfxt.dists[T] = 0.0f;
	
	_build_sfxt(sfxt);
	
	// relax from destinations to super target
	for (auto& p : _endpoints) {
		sfxt.relax(p.vert.id, T, std::nullopt, 0.0f);
	}

	// relax from super source to sources
	for (auto& [src, w] : sfxt.srcs) {
		w = 0.0f;
		sfxt.relax(S, src, std::nullopt, *w);
	}

}

Sfxt Ink::_sfxt_cache(const Point& p) const {
	auto S = _vptrs.size();
	auto to = p.vert.id;

	Sfxt sfxt(S, to, S + 1);

	assert(!sfxt.dists[to]);
	sfxt.dists[to] = 0.0f;
	
	// calculate shortest path tree with dynamic programming
	_build_sfxt(sfxt);

	// relax from super source to sources
	for (auto& [src, w] : sfxt.srcs) {
		w = 0.0f;
		sfxt.relax(S, src, std::nullopt, *w);
	}

	return sfxt;
}


Pfxt Ink::_pfxt_cache(const Sfxt& sfxt) const {
	Pfxt pfxt(sfxt);
	assert(sfxt.dist());

	// generate path prefix from each source vertex
	for (const auto& [src, w] : sfxt.srcs) {
		if (!w || !sfxt.dists[src]) {
			continue;
		}

		auto cost = *sfxt.dists[src] + *w;
		auto node = pfxt.push(cost, sfxt.S, src, nullptr, nullptr, std::nullopt);
		
		// cache pfxt src nodes
		pfxt.srcs.emplace_back(node);	
	}
	return pfxt;
}

void Ink::_update_pfxt(Pfxt& pfxt) {

	// sort affected pfxt nodes by level (ascending)
	auto& ap = _affected_pfxtnodes;
	if (ap.empty()) {
		return;
	}	

	std::sort(ap.begin(), ap.end(), [](PfxtNode* a, PfxtNode* b) {
		return a->level < b->level;	
	});

	//for (auto p : ap) {
	//	std::cout << "lvl=" << p->level << " e=" << p->edge->name() << " removed? " << belongs_to_sfxt[p->edge->id][0] << '\n';
	//}

	// iterate through the affected nodes
	std::queue<PfxtNode*> q;
	auto& sfxt = *_global_sfxt;
	for (auto p : ap) {

		if (p->updated || p->removed) {
			continue;
		}

		q.push(p);
		while (!q.empty()) {
			bool skip{false};
			auto node = q.front();
			q.pop();
			
			if (node->removed) {
				skip = true;
			}

			if (!skip) {
				auto [eptr, w_sel] = _decode_edge(*node->link);
				if (!belongs_to_sfxt[eptr->id][w_sel]) {
					// update cost
					auto v = node->to;
					auto u = node->from;
					auto w = *eptr->weights[w_sel];
					auto detour_cost = *sfxt.dists[v] + w - *sfxt.dists[u];
					node->cost = detour_cost + node->parent->cost;
					node->updated = true;
					node->parent->pruned.emplace_back(eptr->id, w_sel);
				}
				else {
					node->removed = true;
					// add this node's parent to the re-spur list
					if (!node->parent->to_respur) {
						_respurs.emplace_back(node->parent, std::move(node->parent->pruned));
					}
				}
			}

			for (auto c : node->children) {
				q.push(c);
				if (node->removed) {
					c->removed = true;
				}
			}

		}
	}


	// iterate through the re-spur list
	// and spur each node with their
	// corresponding pruned edge + weights
	for (const auto& r : _respurs) {
		_spur_pruned(pfxt, *r.node, r.pruned);	
	}


}

//void Ink::_identify_leaders(
//	PfxtNode* curr,
//	std::vector<PfxtNode*>& dfs_full,
//	std::vector<PfxtNode*>& dfs_marked,
//	size_t& order) {
//
//	// record full dfs traversal
//	dfs_full.emplace_back(curr);
//
//	// we push a node into the euler tour
//	// under 3 scenarios:
//	// 1. edge is removed (edge pointer == null)
//	// 2. edge link belongs to sfxt 
//	//		(since this node is in old prefix tree, we know
//	//		it turned from pfxt node into a sfxt node)
//	// 3. edge is marked as modified by the user AND
//	//		the edge link DOES NOT belong to sfxt, then we know
//	//		this node remains a pfxt node
//	auto [eptr, w_sel] = _decode_edge(*curr->link);
//
//	// NOTE:
//	// CAUTION! do not use node.edge here
//	// because it is not updated when remove_edge is called
//	// we need to check the eptrs mapping
//	if (eptr == nullptr ||
//			_belongs_to_sfxt[eptr->id][w_sel] ||
//			(eptr->modified && !_belongs_to_sfxt[eptr->id][w_sel])) {
//		dfs_marked.emplace_back(curr);
//	}
//
//	curr->subtree_beg = order++;
//
//	// traverse each children
//	for (auto c : curr->children) {
//		_identify_leaders(c, dfs_full, dfs_marked, order);
//	}
//
//
//	if (!dfs_marked.empty()) {
//		auto last = dfs_marked.back();
//		if (last == curr) {
//			if (eptr == nullptr || _belongs_to_sfxt[eptr->id][w_sel]) {
//				// this node will not exist in the new pfxt 
//				// store all its children node as leader nodes
//				for (auto c : curr->children) {
//					auto [e, wsel] = _decode_edge(*c->link);
//					_leaders[e->id][wsel] = c;
//					_leader_cnt++;
//				}
//			}
//			else if (eptr->modified && !_belongs_to_sfxt[eptr->id][w_sel]) {
//				_leaders[eptr->id][w_sel] = curr;
//				_leader_cnt++;
//			}
//		}
//	}
//
//	curr->subtree_end = order;
//}

//void Ink::_identify_leaders_upward() {
//	_leader_nodes.clear();
//	auto& ap = _affected_pfxtnodes;
//	if (ap.empty()) {
//		return;
//	}	
//
//	// sort affected pfxt nodes by level
//	// in descending order
//	std::sort(ap.begin(), ap.end(), [](PfxtNode* a, PfxtNode* b) {
//		return a->level > b->level;	
//	});
//
//	// iterate through affected pfxt nodes
//	for (auto n : ap) {
//		auto [eptr, w_sel] = _decode_edge(*n->link);
//		// if not visited by leader
//		// this node is guaranteed to be a leader
//		if (!n->visited_by_leader) {
//
//			// populate leader lookup table entry
//			if (!_belongs_to_sfxt[eptr->id][w_sel]) {
//				// this node may appear in the new pfxt
//				_leaders[eptr->id][w_sel] = n;
//				_leader_nodes.emplace_back(n);
//			}
//			else {
//				// this node would never exist in the new pfxt
//				// but its children may appear
//				for (auto c : n->children) {
//					auto [e, ws] = _decode_edge(*c->link);
//					_leaders[e->id][ws] = c;
//					_leader_nodes.emplace_back(c);
//				}
//			}
//
//			// upward traversal through parent, grandparent, etc.
//			// any node reachable is definitely not a leader
//			auto curr = n;
//			while (curr->parent) {
//				if (curr->parent->visited_by_leader) {
//					break;
//				}
//				curr->parent->visited_by_leader = true;
//				curr = curr->parent;
//			}
//		}
//		else {
//			// clear leader lookup table entry
//			_leaders[eptr->id][w_sel] = nullptr;
//		}
//	}
//}



void Ink::_propagate_subtree(
	PfxtNode* leader, 
	PfxtNode* node,
	Pfxt& pfxt) {

	// NOTE: only push 1 level
	auto& sfxt = *_global_sfxt;
	for (auto c : leader->children) {
		auto [eptr, w_sel] = _decode_edge(*c->link);
		
		// since c has a leader parent
		// c itself is also a leader
		if (c->num_children() != 0) {
			_leaders[eptr->id][w_sel] = c;
		}

		// detour cost
		auto v = c->to;
		auto u = c->from;
		auto w = *eptr->weights[w_sel];
		auto detour_cost = *sfxt.dists[v] + w - *sfxt.dists[u];
		
		auto cost = detour_cost + node->cost;
		auto e = c->edge;
		auto l = c->link;

		pfxt.push(cost, u, v, e, node, l);	
	}
}


void Ink::_spur_incsfxt(
	size_t K, 
	PathHeap& heap, 
	Pfxt& pfxt, 
	bool save_pfxt_nodes,
	bool recover_paths) {
	auto& sfxt = *_global_sfxt;
	auto beg = std::chrono::steady_clock::now();
	while (!pfxt.num_nodes() == 0) {
		auto node = pfxt.pop();
			
		// no more paths to generate
		if (node == nullptr) {
			break;
		}		

		if (_all_paths.size() >= K || _all_path_costs.size() >= K) {
			break;
		}

		if (recover_paths) {
			// recover the complete path
			auto path = std::make_unique<Path>(0.0f, nullptr);
			_recover_path(*path, sfxt, node, sfxt.T);
			path->weight = path->back().dist;
			if (path->size() > 1) {
				_all_paths.push_back(std::move(path));
			}
		}
		else {
			_all_path_costs.push_back(node->cost);
		}

		// expand search space
		_spur(pfxt, *node);
	}

	auto end = std::chrono::steady_clock::now();
	_elapsed_time_sploop = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count(); 

	if (save_pfxt_nodes) {
		_pfxt_srcs = std::move(pfxt.srcs);
		_pfxt_nodes = std::move(pfxt.nodes);
		_pfxt_paths = std::move(pfxt.paths);
	}	
	
}

void Ink::_spur_incremental(
	size_t K, 
	Pfxt& pfxt,
	bool save_pfxt_nodes,
	bool use_leaders,
	bool recover_paths) {
	
	_update_pfxt(pfxt);
	
	auto& sfxt = *_global_sfxt;
	while (!pfxt.num_nodes() == 0) {
		auto node = pfxt.pop();
		// no more paths to generate
		if (node == nullptr) {
			break;
		}

		if (_all_paths.size() >= K || _all_path_costs.size() >= K) {
			break;
		}


		if (recover_paths) {
			// recover the complete path
			auto path = std::make_unique<Path>(0.0f, nullptr);
			_recover_path(*path, sfxt, node, sfxt.T);
			path->weight = path->back().dist;
			_all_paths.push_back(std::move(path));
		}
		else {
			_all_path_costs.push_back(node->cost);
		}

		// expand search space
		// find children (more detours) for node
		_spur(pfxt, *node);
	}

	if (save_pfxt_nodes) {
		_pfxt_srcs = std::move(pfxt.srcs);
		_pfxt_nodes = std::move(pfxt.nodes);
		_pfxt_paths = std::move(pfxt.paths);
	}

} 


void Ink::_spur_rebuild(size_t K, PathHeap& heap) {
	_sfxt_rebuild();
	auto& sfxt = *_global_sfxt;
	auto pfxt = _pfxt_cache(sfxt);

	auto beg = std::chrono::steady_clock::now();
	while (!pfxt.num_nodes() == 0) {
		auto node = pfxt.pop();
		// no more paths to generate
		if (node == nullptr) {
			break;
		}		

		if (heap.size() >= K) {
			break;
		}

		// recover the complete path
		auto path = std::make_unique<Path>(0.0f, nullptr);
		_recover_path(*path, sfxt, node, sfxt.T);

		path->weight = path->back().dist;
		if (path->size() > 1) {
			heap.push(std::move(path));
			heap.fit(K);
		}

		// expand search space
		_spur(pfxt, *node);
	}
	auto end = std::chrono::steady_clock::now();
	auto elapsed = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
	_elapsed_time_sploop += elapsed;
} 

void Ink::_spur(Point& endpt, size_t K, PathHeap& heap) {
	auto sfxt = _sfxt_cache(endpt);
	auto pfxt = _pfxt_cache(sfxt);

	for (size_t k = 0; k < K; k++) {
		auto node = pfxt.pop();	
		// no more paths to generate
		if (node == nullptr) {
			break;
		}		


		if (heap.size() >= K) {
			break;
		}

		// recover the complete path
		auto path = std::make_unique<Path>(0.0f, &endpt);
		_recover_path(*path, sfxt, node, sfxt.T);
		
		path->weight = path->back().dist;
		if (path->size() > 1) {
			heap.push(std::move(path));
			heap.fit(K);
		}

		// expand search space
		_spur(pfxt, *node);
	}

} 

void Ink::_spur(Pfxt& pfxt, PfxtNode& pfx) {
	auto u = pfx.to;

	while (u != pfxt.sfxt.T) {
		//assert(pfxt.sfxt.links[u]);
		auto uptr = _vptrs[u];

		for (auto edge : uptr->fanout) {
			for (size_t w_sel = 0; w_sel < NUM_WEIGHTS; w_sel++) {
				if (!edge->weights[w_sel]) {
					continue;
				}

				// skip if the edge goes outside of the suffix tree
				// which is unreachable
				auto v = edge->to.id;
				if (!pfxt.sfxt.dists[v]) {
					continue;
				}

				// skip if the edge belongs to the suffix 
				// NOTE: we're detouring, so we don't want to
				// go on the same paths explored by the suffix tree
				if (pfxt.sfxt.links[u] &&
						_encode_edge(*edge, w_sel) == *pfxt.sfxt.links[u]) {
					continue;
				}
			
				auto w = *edge->weights[w_sel];
				auto detour_cost = *pfxt.sfxt.dists[v] + w - *pfxt.sfxt.dists[u]; 
				
				auto c = detour_cost + pfx.cost;
				pfxt.push(c, u, v, edge, &pfx, _encode_edge(*edge, w_sel));
			}
		}

		u = *pfxt.sfxt.successors[u];
	}
}

void Ink::_spur_pruned(
	Pfxt& pfxt, 
	PfxtNode& pfx,
	const std::vector<std::pair<size_t, size_t>>& pruned) {
	auto u = pfx.to;

	// mark pruned edges on graph
	for (const auto& p : pruned) {
		_eptrs[p.first]->pruned_weights[p.second] = true;
	}

	while (u != pfxt.sfxt.T) {
		//assert(pfxt.sfxt.links[u]);
		auto uptr = _vptrs[u];

		for (auto edge : uptr->fanout) {
			for (size_t w_sel = 0; w_sel < NUM_WEIGHTS; w_sel++) {
				if (!edge->weights[w_sel]) {
					continue;
				}

				// skip if the edge goes outside of the suffix tree
				// which is unreachable
				auto v = edge->to.id;
				if (!pfxt.sfxt.dists[v]) {
					continue;
				}

				// skip if the edge belongs to the suffix 
				// NOTE: we're detouring, so we don't want to
				// go on the same paths explored by the suffix tree
				if (pfxt.sfxt.links[u] &&
						_encode_edge(*edge, w_sel) == *pfxt.sfxt.links[u]) {
					continue;
				}

				// skip if the [edge id, weight sel] pair is marked as pruned
				if (edge->pruned_weights[w_sel]) {
					continue;
				}

				auto w = *edge->weights[w_sel];
				auto detour_cost = *pfxt.sfxt.dists[v] + w - *pfxt.sfxt.dists[u]; 
				
				auto c = detour_cost + pfx.cost;
				pfxt.push(c, u, v, edge, &pfx, _encode_edge(*edge, w_sel));
			}
		}

		u = *pfxt.sfxt.successors[u];
	}

	// reset markings on pruned edges
	// NOTE: they're only pruned when we spur this
	// particular pfxt node
	for (const auto& p : pruned) {
		_eptrs[p.first]->pruned_weights[p.second] = false;
	}

	// TODO: finish the pfxt validation phase here

}


void Ink::_recover_path(
	Path& path,
	const Sfxt& sfxt,
	const PfxtNode* pfxt_node,
	size_t v) const {

	if (pfxt_node == nullptr) {
		return;
	}

	// recurse until we reach the source pfxt node
	_recover_path(path, sfxt, pfxt_node->parent, pfxt_node->from);

	auto u = pfxt_node->to;
	auto u_ptr = _vptrs[u];

	// detour at the sfxt source
	if (pfxt_node->from == sfxt.S) {
		path.emplace_back(*u_ptr, 0.0f);
	}
	// detour at non-sfxt-source nodes (internal deviation)
	else {
		assert(!path.empty());
		assert(pfxt_node->link);
		
		auto [edge, w_sel] = _decode_edge(*pfxt_node->link);
		auto d = path.back().dist + *edge->weights[w_sel];
		path.emplace_back(*u_ptr, d);
	}
	
	while (u != v) {
		if (!sfxt.links[u]) {
			break;
		}
		
		assert(sfxt.links[u]);
		auto [edge, w_sel] = _decode_edge(*sfxt.links[u]);	
		// move to the successor of u
		u = *sfxt.successors[u];
		u_ptr = _vptrs[u];
		
		auto d = path.back().dist + *edge->weights[w_sel];
		path.emplace_back(*u_ptr, d);
	}

}	

void Ink::_clear_update_list() {
	while (!_to_update.empty()) {
		auto v = _to_update.back();
		if (_vptrs[v]) {
			_vptrs[v]->is_in_update_list = false;
		}
		_to_update.pop_back();
	}
}

void Ink::_dfs(
	size_t v, 
	std::deque<size_t>& tpg, 
	std::vector<bool>& visited) {
	visited[v] = true;
	auto vptr = _vptrs[v];
	for (const auto e : vptr->fanin) {
		auto u = e->from.id;
		if (!visited[u]) {
			_dfs(u, tpg, visited);
		}
	}

	tpg.push_front(v);
}

std::vector<Path> Ink::_extract_paths(
	std::vector<std::unique_ptr<Path>>& paths) {
	std::sort(
		paths.begin(), 
		paths.end(), 
		[](std::unique_ptr<Path>& a, std::unique_ptr<Path>& b) {
			return a->weight < b->weight;	
		}
	);

	std::vector<Path> P;
	P.reserve(paths.size());

	std::transform(
		paths.begin(), paths.end(), std::back_inserter(P), 
		[](auto& ptr) {
			return std::move(*ptr);		
		}
	);

	paths.clear();
	return P;
}



// ------------------------
// Suffix Tree Implementations
// ------------------------

// NOTE:
// OpenTimer does a resize_to_fit, why not simply resize(N)?

Sfxt::Sfxt(size_t S, size_t T, size_t sz) :
	S{S},
	T{T}
{
	// resize suffix tree storages
	visited.resize(sz);
	dists.resize(sz);
	successors.resize(sz);
	links.resize(sz);
}

inline bool Sfxt::relax(
	size_t u, 
	size_t v, 
	std::optional<std::pair<size_t, size_t>> l, 
	float d) {
	
	if (!dists[u] || *dists[v] + d < *dists[u]) {
		dists[u] = *dists[v] + d;
		successors[u] = v;
		links[u] = l; 
		return true;
	}
	return false;
}


inline std::optional<float> Sfxt::dist() const {
	return dists[S];
}


// ------------------------
// Prefix Tree Implementations
// ------------------------
PfxtNode::PfxtNode(
	float c, 
	size_t f, 
	size_t t, 
	Edge* e,
	PfxtNode* p,
	std::optional<std::pair<size_t, size_t>> l) :
	cost{c},
	from{f},
	to{t},
	edge{e},
	parent{p},
	link{l}
{ 
}

size_t PfxtNode::num_children() const {
	return children.size();
}

Pfxt::Pfxt(const Sfxt& sfxt) : 
	sfxt{sfxt} 
{
}

Pfxt::Pfxt(Pfxt&& other) :
	sfxt{other.sfxt},
	comp{other.comp},
	paths{std::move(other.paths)},
	nodes{std::move(other.nodes)}
{
}

PfxtNode* Pfxt::push(
	float c,
	size_t f,
	size_t t,
	Edge* e,
	PfxtNode* p,
	std::optional<std::pair<size_t, size_t>> link) {
	nodes.emplace_back(std::make_unique<PfxtNode>(c, f, t, e, p, link));

	// cache the pointer we just pushed
	auto obs = nodes.back().get();

	// store this node to its corresponding edge's
	// dependency list
	if (e) {
		e->dep_nodes.emplace_back(obs);	
	}

	// store children
	if (p) {
		p->children.emplace_back(obs);
		obs->level = p->level + 1;
	}
	else {
		obs->level = 0;	
	}

	// heapify nodes
	std::push_heap(nodes.begin(), nodes.end(), comp);
	
	return obs;
}



PfxtNode* Pfxt::pop() {
	if (nodes.empty()) {
		return nullptr;
	}
	
	// swap [0] and [N-1], and heapify [first, N-1)
	std::pop_heap(nodes.begin(), nodes.end(), comp);

	// get the min element from heap
	// now it's located in the back
	paths.push_back(std::move(nodes.back()));
	nodes.pop_back();

	// and return an observer poiner to this node object
	return paths.back().get();
}


PfxtNode* Pfxt::top() const {
	return nodes.empty() ? nullptr : nodes.front().get();
}


// ------------------------
// PfxtNodeComp Implementations
// ------------------------
bool Pfxt::PfxtNodeComp::operator() (
	const std::unique_ptr<PfxtNode>& a,
	const std::unique_ptr<PfxtNode>& b) const {
	return a->cost > b->cost;
}	

// ------------------------
// Point Implementations
// ------------------------
Point::Point(const Vert& v, float d) :
	vert{v},
	dist{d}
{
}

// ------------------------
// Path Implementations
// ------------------------
Path::Path(float w, const Point* endpt) :
	weight{w},
	endpoint{endpt}
{
}

void Path::dump(std::ostream& os) const {
	if (empty()) {
		os << "empty path\n";
	}

	// dump the header
	os << "----------------------------------\n";
	os << "Startpoint:  " << front().vert.name << '\n';
	os << "Endpoint:    " << back().vert.name << '\n';

	// dump the path
	os << "Path:\n";
	for (const auto& p : *this) {
		os << "Vert name: " << p.vert.name << ", Dist: " << p.dist << '\n';
	}

	os << "----------------------------------\n";
}

// ------------------------
// PathHeap Implementations
// ------------------------
inline size_t PathHeap::size() const {
	return _paths.size();
}

inline bool PathHeap::empty() const {
	return _paths.empty();
}

void PathHeap::push(std::unique_ptr<Path> path) {
	_paths.push_back(std::move(path));
	std::push_heap(_paths.begin(), _paths.end(), _comp);
}

void PathHeap::pop() {
	if (_paths.empty()) {
		return;
	}

	std::pop_heap(_paths.begin(), _paths.end(), _comp);
	_paths.pop_back();
}

Path* PathHeap::top() const {
	return _paths.empty() ? nullptr : _paths.front().get();
}

void PathHeap::heapify() {
	std::make_heap(_paths.begin(), _paths.end(), _comp);
} 

void PathHeap::fit(size_t K) {
	while (_paths.size() > K) {
		pop();
	}
}

void PathHeap::merge_and_fit(PathHeap&& rhs, size_t K) {
	if (_paths.capacity() < rhs._paths.capacity()) {
		_paths.swap(rhs._paths);
	}

	std::sort_heap(_paths.begin(), _paths.end(), _comp);
	std::sort_heap(rhs._paths.begin(), rhs._paths.end(), _comp);

	// now insert rhs to the back of _paths
	auto mid = _paths.insert(
		_paths.end(),
		std::make_move_iterator(rhs._paths.begin()),
		std::make_move_iterator(rhs._paths.end())
	);

	// rhs elements are invalidated
	rhs._paths.clear();

	std::inplace_merge(_paths.begin(), mid, _paths.end(), _comp);
	if (_paths.size() > K) {
		_paths.resize(K);
	}

	heapify();

}


std::vector<Path> PathHeap::extract() {
	std::sort_heap(_paths.begin(), _paths.end(), _comp);
	std::vector<Path> P;
	P.reserve(_paths.size());

	std::transform(
		_paths.begin(), _paths.end(), std::back_inserter(P), 
		[](auto& ptr) {
			return std::move(*ptr);		
		}
	);

	_paths.clear();
	return P;
}


void PathHeap::dump(std::ostream& os) const {
	os << "Num Paths = " << _paths.size() << '\n';
	for (const auto& p : _paths) {
		p->dump(os);
	}
}


} // end of namespace ink 

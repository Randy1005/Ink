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

		// stor, truee pointer to this edge object
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

std::vector<Path> Ink::report_incsfxt(size_t K, bool save_pfxt_nodes) {
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
	_spur_incsfxt(K, heap, pfxt, save_pfxt_nodes);	
	auto end = std::chrono::steady_clock::now();
	auto elapsed = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
	_elapsed_time_spur = elapsed;
	
	// report complete, clear the update list
	_clear_update_list();		
	
	beg = std::chrono::steady_clock::now();
	auto paths = heap.extract();
	end = std::chrono::steady_clock::now();
	elapsed = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
	_elapsed_final_path_sort += elapsed;
	return paths;
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
	bool use_leaders) {
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
	_spur_incremental(K, heap, pfxt, save_pfxt_nodes, use_leaders);	
	auto end = std::chrono::steady_clock::now();
	_elapsed_time_spur = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();

	// report complete, clear the update list
	_clear_update_list();		


	beg = std::chrono::steady_clock::now();
	auto paths = _extract_paths(_all_paths);
	end = std::chrono::steady_clock::now();
	_elapsed_final_path_sort = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
	return paths;
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

void Ink::dump_profile(std::ostream& os, bool reset) {
	os << "======== Profile ========\n";
	os << "full spur function: " << _elapsed_time_spur << " ns ("
		 << _elapsed_time_spur / 1000000000.0f << " sec)\n";
	os << "full spur function 2: " << _elapsed_time_spur2 << " ns ("
		 << _elapsed_time_spur2 / 1000000000.0f << " sec)\n";
	//os << "identify leader: " << _elapsed_time_idl << " ns ("
	//	 << _elapsed_time_idl / 1000000000.0f << " sec)\n";
	//os << "spur loop: " << _elapsed_time_sploop << " ns ("
	//	 << _elapsed_time_sploop / 1000000000.0f << " sec)\n";
	//os << "final path sort: " << _elapsed_final_path_sort << " ns ("
	//	 << _elapsed_final_path_sort / 1000000000.0f << " sec)\n";
	//os << "sfxt/pfxt init: " << _elapsed_init << " ns ("
	//	 << _elapsed_init / 1000000000.0f << " sec)\n";
	//
	//os << "pop time: " << _elapsed_pop << " ns ("
	//	 << _elapsed_pop / 1000000000.0f << " sec)\n";

	//os << "search space expansion time: " << _elapsed_sse << " ns ("
	//	 << _elapsed_sse / 1000000000.0f << " sec)\n";
	os << "pfxt node transfer: " << _elapsed_time_tr << " ns ("
		 << _elapsed_time_tr / 1000000000.0f << " sec)\n";
	os << "search space expansions: " << _sses << '\n';
	os << "max nodes: " << _max_nodes << '\n';
	os << "sort cnt: " << _sort_cnt << '\n';
	os << "into precomp: " << _skip_heaps << '\n';
	if (reset) {
		_elapsed_time_spur = 0;
		_elapsed_time_spur2 = 0;
		_elapsed_time_sploop = 0;
		_elapsed_time_idl = 0;
		_elapsed_prop = 0;
		_elapsed_time_tr = 0;
		_elapsed_final_path_sort = 0;
		_elapsed_init = 0;
		_leader_matched = 0;
		_elapsed_sse = 0;
		_sses = 0;
		_elapsed_pop = 0;
		_max_precomp_nodes = 0;
		_max_nodes = 0;
		_sort_cnt = 0;
		_skip_heaps = 0;
	}
	os << "=========================\n\n";
}


void Ink::dump_more_paths(std::ostream& os) {
	auto paths = _extract_paths(_more_paths);

	for (auto& p : paths) {
		os << p.weight << '\n';
	}

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
	_belongs_to_sfxt.clear();
	_belongs_to_sfxt.resize(num_edges());
	
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
			// NOTE: this part is sus, it's probably not marking the sfxt edges right
			// TODO: look at this case later
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
		
			// compare the cached link to the current link
			//if (sfxt.links[v] != old_link) {
			//	// this means another edge took over and became the successor
			//	// or another weight is selected for the same edge
			//	// NOTE: either case we need a vector<array[NUM_WEIGHTS]>
			//	// to mark if a link took over, because multiple pfxt nodes
			//	// may point to the same edge, but uses different weights
			//	auto [eptr, w_sel] = _decode_edge(*sfxt.links[v]);
			//	_belongs_to_sfxt[eptr->id][w_sel] = true;
			//}

			
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


void Ink::_identify_leaders(
	PfxtNode* curr,
	std::vector<PfxtNode*>& dfs_full,
	std::vector<PfxtNode*>& dfs_marked,
	size_t& order) {

	// record full dfs traversal
	dfs_full.emplace_back(curr);

	// we push a node into the euler tour
	// under 3 scenarios:
	// 1. edge is removed (edge pointer == null)
	// 2. edge link belongs to sfxt 
	//		(since this node is in old prefix tree, we know
	//		it turned from pfxt node into a sfxt node)
	// 3. edge is marked as modified by the user AND
	//		the edge link DOES NOT belong to sfxt, then we know
	//		this node remains a pfxt node
	auto [eptr, w_sel] = _decode_edge(*curr->link);

	// NOTE:
	// CAUTION! do not use node.edge here
	// because it is not updated when remove_edge is called
	// we need to check the eptrs mapping
	if (eptr == nullptr ||
			_belongs_to_sfxt[eptr->id][w_sel] ||
			(eptr->modified && !_belongs_to_sfxt[eptr->id][w_sel])) {
		dfs_marked.emplace_back(curr);
	}

	curr->subtree_beg = order++;

	// traverse each children
	for (auto c : curr->children) {
		_identify_leaders(c, dfs_full, dfs_marked, order);
	}


	if (!dfs_marked.empty()) {
		auto last = dfs_marked.back();
		if (last == curr) {
			if (eptr == nullptr || _belongs_to_sfxt[eptr->id][w_sel]) {
				// this node will not exist in the new pfxt 
				// store all its children node as leader nodes
				for (auto c : curr->children) {
					auto [e, wsel] = _decode_edge(*c->link);
					_leaders[e->id][wsel] = c;
				}
			}
			else if (eptr->modified && !_belongs_to_sfxt[eptr->id][w_sel]) {
				_leaders[eptr->id][w_sel] = curr;
			}
		}
	}

	curr->subtree_end = order;
}



void Ink::_propagate_subtree(
	PfxtNode* leader, 
	PfxtNode* node,
	Pfxt& pfxt) {

	// NOTE: only push 1 level
	auto& sfxt = *_global_sfxt;
	auto t = pfxt.top();
	for (auto c : leader->children) {
		auto [eptr, w_sel] = _decode_edge(*c->link);
		
		// since c has a leader parent
		// c itself is also a leader
		_leaders[eptr->id][w_sel] = c;

		// detour cost
		auto v = c->to;
		auto u = c->from;
		auto w = *eptr->weights[w_sel];
		auto detour_cost = *sfxt.dists[v] + w - *sfxt.dists[u];
		
		auto cost = detour_cost + node->cost;
		auto e = c->edge;
		auto l = c->link;

		if (t && cost < t->cost) {
			auto n = std::make_unique<PfxtNode>(cost, u, v, e, node, l);
			auto n_obs = n.get();
			if (node) {
				node->children.emplace_back(n_obs);
			}
			
			// store in top's pre-computed list
			// it will need to be examined before top
			t->precomps.emplace_back(n_obs);

			// accumulate how many paths should be recovered 
			// before moving onto top
			t->precomp_paths += (c->subtree_end - c->subtree_beg);
			
			pfxt.paths.push_back(std::move(n));
		}
		else {
			pfxt.push(cost, u, v, e, node, l);	
		}
	}

	


	//auto& sfxt = *_global_sfxt;
	//auto top = pfxt.top();

	//_bfs_old.clear();	
	//_bfs_old.push_back(leader);
	//_bfs_new.clear();
	//_bfs_new.emplace_back(node);
	//
	//// perform level-by-level traversal
	//// of this subtree
	//bool enough_paths{false};
	//while (!_bfs_old.empty()) {
	//	auto s = _bfs_old.front();
	//	_bfs_old.pop_front();
	//	auto parent = _bfs_new.front();
	//	_bfs_new.pop_front();

	//	for (auto c : s->children) {
	//		
	//		auto [eptr, w_sel] = _decode_edge(*c->link);
	//		_leaders[eptr->id][w_sel] = c;

	//		auto v = c->to;
	//		auto u = c->from;
	//		auto w = *eptr->weights[w_sel];
	//		// calculate detour cost
	//		auto detour_cost = *sfxt.dists[v] + w - *sfxt.dists[u]; 

	//		// calculate all the information to construct a pfxt node
	//		auto cost = detour_cost + parent->cost;
	//		auto f = c->from;
	//		auto t = c->to;
	//		auto e = c->edge;
	//		auto l = c->link;


	//		PfxtNode* p_obs{nullptr};
	//		
	//		//if (top == nullptr || cost > top->cost) {
	//		//	// using the updated info, push a new pfxt node
	//		//	p_obs = pfxt.push(cost, f, t, e, parent, l);
	//		//}
	//		if (top != nullptr || cost < top->cost) {
	//			auto p = std::make_unique<PfxtNode>(cost, f, t, e, parent, l);
	//			p_obs = p.get();
	//			
	//			if (parent) {
	//				parent->children.emplace_back(p_obs);
	//			}

	//			auto path = std::make_unique<Path>(0.0f, nullptr);
	//			_recover_path(*path, sfxt, p_obs, sfxt.T);
	//			path->weight = path->back().dist;
	//			pfxt.paths.push_back(std::move(p));
	//		
	//			_all_paths.push_back(std::move(path));

	//			if (_all_paths.size() >= K) {
	//				enough_paths = true;
	//			}
	//		}

	//		if (enough_paths) {
	//			break;
	//		}


	//
	//		_bfs_old.push_back(c);
	//		_bfs_new.push_back(p_obs);
	//	}

	//}

}

void Ink::_dfs_subtree(
	PfxtNode* parent, 
	PfxtNode* node,
	PfxtNode* top,
	Pfxt& pfxt) {
	
	auto& sfxt = *_global_sfxt;
	auto [eptr, w_sel] = _decode_edge(*node->link);
		
	// detour cost
	auto v = node->to;
	auto u = node->from;
	auto w = *eptr->weights[w_sel];
	auto detour_cost = *sfxt.dists[v] + w - *sfxt.dists[u];
	
	auto cost = detour_cost + parent->cost;
	auto e = node->edge;
	auto l = node->link;

	if (top == nullptr || 
			cost >= top->cost ||
			node->is_in_heap) {
		return;
	}
	
	if (!node->is_in_heap) {
		node->new_node = pfxt.push(cost, u, v, e, parent, l);
		node->is_in_heap = true;
	}

	for (auto c : node->children) {
		_dfs_subtree(node->new_node, c, top, pfxt);
	}

}


void Ink::_spur_incsfxt(size_t K, PathHeap& heap, Pfxt& pfxt, bool save_pfxt_nodes) {
	auto beg = std::chrono::steady_clock::now();
	auto& sfxt = *_global_sfxt;
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
		_sses++;
		_spur(pfxt, *node);
	}


	if (save_pfxt_nodes) {
		_pfxt_srcs = pfxt.srcs;
		_pfxt_nodes = std::move(pfxt.nodes);
		_pfxt_paths = std::move(pfxt.paths);
	}	
	
	auto end = std::chrono::steady_clock::now();
	_elapsed_time_spur2 = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
}

void Ink::_spur_incremental(
	size_t K, 
	PathHeap& heap,
	Pfxt& pfxt,
	bool save_pfxt_nodes,
	bool use_leaders) {
	auto beg = std::chrono::steady_clock::now();
	auto& sfxt = *_global_sfxt;
	// TODO : incremental euler tour?
	if (use_leaders) {
		// resize leader storage size to 
		// be as same as num_edges
		_leaders.clear();
		_leaders.resize(num_edges());

		size_t order{0};
		// we apply dfs to get the lowest affected nodes (defined as leader nodes)
		_dfs_marked.clear();
		_dfs_full.clear();
		for (auto src : _pfxt_srcs) {
			// NOTE: each pfxt src's children should 
			// be marked as modified, this way they 
			// by default lead every node below them
			for (auto c : src->children) {
				auto [eptr, w_sel] = _decode_edge(*c->link);
				if (eptr != nullptr && !_belongs_to_sfxt[eptr->id][w_sel]) {
					eptr->modified = true;
				}
				
				_identify_leaders(c, _dfs_full, _dfs_marked, order);
			}
		}
	}
	
	while (!pfxt.num_nodes() == 0) {
		PfxtNode* node{nullptr};
		auto top = pfxt.top();
		if (top->precomps.size() != 0) {
			_skip_heaps++;
			auto paths_left = K - _all_paths.size();
			if (top->precomp_paths > paths_left) {
				_sort_cnt++;
				std::sort(top->precomps.begin(), top->precomps.end(), 
					[](auto a, auto b) {
						return a->cost < b->cost; 
					}
				);
			}

			node = top->precomps.front();
			top->precomps.pop_front();
			top->precomp_paths--;
		}
		else {
			node = pfxt.pop();
		}


		// no more paths to generate
		if (node == nullptr) {
			break;
		}

		// recover the complete path
		auto path = std::make_unique<Path>(0.0f, nullptr);
		_recover_path(*path, sfxt, node, sfxt.T);
		path->weight = path->back().dist;
		_all_paths.push_back(std::move(path));
		if (_all_paths.size() >= K) {
			break;
		}
	
		// get the edge pointer and weight selection of this node
		if (node->link && use_leaders) {
			auto [eptr, w_sel] = _decode_edge(*node->link);
			if (_leaders[eptr->id][w_sel]) {
				// we recorded a leader from previous report
				// recover its children prefix tree nodes
				auto l = _leaders[eptr->id][w_sel];
				_propagate_subtree(l, node, pfxt);
			}
		}

		if (node->num_children() != 0) {
			continue;
		}

		// expand search space
		// find children (more detours) for node
		_sses++;
		_spur(pfxt, *node);
	}
	
	if (save_pfxt_nodes) {
		_pfxt_srcs = std::move(pfxt.srcs);
		_pfxt_nodes = std::move(pfxt.nodes);
		_pfxt_paths = std::move(pfxt.paths);
	}

	// TODO: will need to reset somehow
	// reset edge modified flag
	//for (auto e : _eptrs) {
	//	if (e) {
	//		e->modified = false;
	//	}
	//}
	
	auto end = std::chrono::steady_clock::now();
	_elapsed_time_spur2 = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg).count();
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
		auto u_ptr = _vptrs[u];

		for (auto edge : u_ptr->fanout) {
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

	// store a raw pointer to this node as parent's children
	if (p != nullptr) {
		p->children.emplace_back(obs);
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

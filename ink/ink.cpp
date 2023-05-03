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


// ------------------------
// Ink Implementations
// ------------------------

void Ink::read_ops_and_report(const std::string& in, const std::string& out) {
	std::ifstream ifs;
	ifs.open(in);
	if (!ifs) {
		throw std::runtime_error("Failed to open file: " + in);
	}

	std::ofstream ofs(out);

	_read_ops_and_report(ifs, ofs);
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
		if (!_vptrs[e->from.id]->is_in_update_list) {
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
		if (!_vptrs[e->to.id]->is_in_update_list) {
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


	//auto itr = std::find_if(_edges.begin(), _edges.end(), [&](const Edge& e) {
	//	return (e.from.name == from) && (e.to.name == to);
	//});


	if (itr != _name2eit.end()) {
		// edge exists
		auto& e = *itr->second;

		// update weights
		for (size_t i = 0; i < e.weights.size(); i++) {
			e.weights[i] = ws[i];
		}
			
		// record the to vertex of this edge
		// when we do incremental report, we
		// need to perform relaxation on this
		// vertex again
		if (!_vptrs[e.to.id]->is_in_update_list) {
			_to_update.emplace_back(e.to.id);
			_vptrs[e.to.id]->is_in_update_list = true;
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

		// store pointer to this edge object
		_eptrs[id] = &e;

		// update fanin, fanout
		// cache fanin, fanout satellite iterators
		// (for edge removal)
		e.from.insert_fanout(e);
		e.to.insert_fanin(e);

		// update the edge name to iterator mapping
		_name2eit.emplace(ename, _edges.begin());
		
		// record the to vertex of this edge
		// when we do incremental report, we
		// need to perform relaxation on this
		// vertex again
		if (!_vptrs[e.to.id]->is_in_update_list) {
			_to_update.emplace_back(e.to.id);
			_vptrs[e.to.id]->is_in_update_list = true;
		}
		return e; 
	}


}

void Ink::remove_edge(const std::string& from, const std::string& to) {
  
	const std::string ename = from + "->" + to;
	auto itr = _name2eit.find(ename);
	
	//auto itr = std::find_if(_edges.begin(), _edges.end(), [&](const Edge& e) {
	//	return e.from.name == from && e.to.name == to;
	//});

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

	return heap.extract();
}

std::vector<Path> Ink::report_global(size_t K) {
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
	_spur_global(K, heap);	
	
	
	return heap.extract();
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

void Ink::_read_ops_and_report(std::istream& is, std::ostream& os) {
	std::string buf;
	while (true) {
		is >> buf;
		if (is.eof()) {
			break;
		}
		
		if (buf == "report") {
			is >> buf;
			auto paths = report(std::stoul(buf));
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
	// record both from and to of this edge
	// they both need to be re-relaxed when we
	// run incremental report
	if (!_vptrs[e.from.id]->is_in_update_list) {
		_to_update.emplace_back(e.from.id);
		_vptrs[e.from.id]->is_in_update_list = true;
	}

	if (!_vptrs[e.to.id]->is_in_update_list) {
		_to_update.emplace_back(e.to.id);
		_vptrs[e.to.id]->is_in_update_list = true;
	}
	
	
	// update fanout of v_from, fanin of v_to
	e.from.remove_fanout(e);
	e.to.remove_fanin(e);

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
	if (sfxt.S >= sfxt.T) {
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
	auto S = _vptrs.size();
	auto T = _vptrs.size() + 1;
	
	_global_sfxt = Sfxt(S, T);
	assert(!_global_sfxt.dists[T]);
	_global_sfxt.dists[T] = 0.0f;

	// relax from destinations to super target
	for (auto& p : _endpoints) {
		_global_sfxt.relax(p.vert.id, T, std::nullopt, 0.0f);
	}

	_build_sfxt(_global_sfxt);
	
	// relax from super source to sources
	for (auto& [src, w] : _global_sfxt.srcs) {
		w = 0.0f;
		_global_sfxt.relax(S, src, std::nullopt, *w);
	}

}

Ink::Sfxt Ink::_sfxt_cache(const Point& p) const {
	auto S = _vptrs.size();
	auto to = p.vert.id;

	Sfxt sfxt(S, to);

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


Ink::Pfxt Ink::_pfxt_cache(const Sfxt& sfxt) const {
	Pfxt pfxt(sfxt);
	assert(sfxt.dist());

	// generate path prefix from each source vertex
	for (const auto& [src, w] : sfxt.srcs) {
		if (!w || !sfxt.dists[src]) {
			continue;
		}

		auto s = *sfxt.dists[src] + *w;
		pfxt.push(s, sfxt.S, src, nullptr, nullptr, std::nullopt);
	}
	return pfxt;
}

void Ink::_spur_global(size_t K, PathHeap& heap) {
	_sfxt_cache();
	auto pfxt = _pfxt_cache(_global_sfxt);

//	for (size_t k = 0; k < K; k++) {
//		auto node = pfxt.pop();
//		// no more paths to generate
//		if (node == nullptr) {
//			break;
//		}		
//
//		if (heap.size() >= K) {
//			break;
//		}
//
//		// recover the complete path
//		auto path = std::make_unique<Path>(0.0f, nullptr);
//		_recover_path(*path, _global_sfxt, node, _global_sfxt.T);
//
//		path->weight = path->back().dist;
//		if (path->size() > 1) {
//			heap.push(std::move(path));
//			heap.fit(K);
//		}
//
//		// expand search space
//		_spur(pfxt, *node);
//	}

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
		_recover_path(*path, _global_sfxt, node, _global_sfxt.T);

		path->weight = path->back().dist;
		if (path->size() > 1) {
			heap.push(std::move(path));
			heap.fit(K);
		}

		// expand search space
		_spur(pfxt, *node);
	}

} 

void Ink::_spur(Point& endpt, size_t K, PathHeap& heap) const {
	auto sfxt = _sfxt_cache(endpt);
	auto pfxt = _pfxt_cache(sfxt);

	for (size_t k = 0; k < K; k++) {
		auto node = pfxt.pop();	
		// no more paths to generate
		if (node == nullptr) {
			break;
		}		

		// NOTE: question ...
		// why do we stop when max-weight path in heap 
		// is smaller than pfxt node weight?
		// NOTE: is it because there's no chance of finding a path with
		// less weight?
		if (heap.size() >= K/* && heap.top()->weight <= node->weight */) {
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

void Ink::_spur(Pfxt& pfxt, const PfxtNode& pfx) const {
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
				
				auto s = detour_cost + pfx.detour_cost;
				pfxt.push(s, u, v, edge, &pfx, _encode_edge(*edge, w_sel));
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
	auto u_vptr = _vptrs[u];

	// detour at the sfxt source
	if (pfxt_node->from == sfxt.S) {
		path.emplace_back(*u_vptr, 0.0f);
	}
	// detour at non-sfxt-source nodes (internal deviation)
	else {
		assert(!path.empty());
		assert(pfxt_node->encoded_edge);
		
		auto [edge, w_sel] = _decode_edge(*pfxt_node->encoded_edge);
		auto d = path.back().dist + *edge->weights[w_sel];
		path.emplace_back(*u_vptr, d);
	}
	
	while (u != v) {
		if (!sfxt.links[u]) {
			break;
		}
		
		assert(sfxt.links[u]);
		auto [edge, w_sel] = _decode_edge(*sfxt.links[u]);	
		// move to the successor of u
		u = *sfxt.successors[u];
		u_vptr = _vptrs[u];
		
		auto d = path.back().dist + *edge->weights[w_sel];
		path.emplace_back(*u_vptr, d);
	}

}	


// ------------------------
// Suffix Tree Implementations
// ------------------------

// NOTE:
// OpenTimer does a resize_to_fit, why not simply resize(N)?

Ink::Sfxt::Sfxt(size_t S, size_t T) :
	S{S},
	T{T}
{
	// resize suffix tree storages
	size_t sz = std::max(S, T) + 1;
	visited.resize(sz);
	dists.resize(sz);
	successors.resize(sz);
	links.resize(sz);
}

inline bool Ink::Sfxt::relax(
	size_t u, 
	size_t v, 
	std::optional<size_t> e, 
	float d) {
	
	if (!dists[u] || *dists[v] + d < *dists[u]) {
		dists[u] = *dists[v] + d;
		successors[u] = v;
		links[u] = e; 
		return true;
	}
	return false;
}


inline std::optional<float> Ink::Sfxt::dist() const {
	return dists[S];
}


// ------------------------
// Prefix Tree Implementations
// ------------------------
Ink::PfxtNode::PfxtNode(
	float c, 
	size_t f, 
	size_t t, 
	const Edge* e,
	const PfxtNode* p,
	std::optional<size_t> encoded_e) :
	detour_cost{c},
	from{f},
	to{t},
	edge{e},
	parent{p},
	encoded_edge{encoded_e}
{ 
}

Ink::Pfxt::Pfxt(const Sfxt& sfxt) : sfxt{sfxt} 
{
}

Ink::Pfxt::Pfxt(Pfxt&& other) :
	sfxt{other.sfxt},
	comp{other.comp},
	paths{std::move(other.paths)},
	nodes{std::move(other.nodes)}
{
}

void Ink::Pfxt::push(
	float w,
	size_t f,
	size_t t,
	const Edge* e,
	const PfxtNode* p,
	std::optional<size_t> encoded_e) {
	nodes.emplace_back(std::make_unique<PfxtNode>(w, f, t, e, p, encoded_e));
	// heapify nodes
	std::push_heap(nodes.begin(), nodes.end(), comp);
}


Ink::PfxtNode* Ink::Pfxt::pop() {
	if (nodes.empty()) {
		return nullptr;
	}
	
	// swap [0] and [N-1], and heapify [first, N-1)
	std::pop_heap(nodes.begin(), nodes.end(), comp);

	// get the min element from heap
	// now it's located in the back
	// NOTE: ownership is transferred to paths
	paths.push_back(std::move(nodes.back()));
	nodes.pop_back();
	// and return a poiner to this node object
	return paths.back().get();	
}

Ink::PfxtNode* Ink::Pfxt::top() const {
	return nodes.empty() ? nullptr : nodes.front().get();
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
	
	// NOTE: question ...
	// why do I want the one with bigger capacity in front?
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

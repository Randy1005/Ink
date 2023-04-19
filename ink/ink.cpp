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

/*
// TODO: understand the following problem
{
auto a = std::make_unique<int>(5);
}

{
	auto a = std::unique_ptr<int>(new int(5));

}

*/

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

void Ink::read_graph(const std::string& file) {
	std::ifstream ifs;
	ifs.open(file);
	if (!ifs) {
		throw std::runtime_error("Failed to open file: " + file);
	}

	_read_graph(ifs);
}


Vert& Ink::insert_vertex(const std::string& name) {
	// TODO: correct this code
	
	// NOTE: 
	// 1. (vertex non-existent): simply insert
	// if a valid id can be popped from the free list, assign
	// it to this new vertex
	// if no free id is available, increment the size of storage
	// 2. (vertex exists): do nothing

	// check if a vertex with this name exists
	auto itr = _name2v.find(name);

	// TODO: study piece-wise construct
	if (itr == _name2v.end()) {
		// store in name to object map
		auto id = _idxgen_vert.get();
		auto [iter, success] = _name2v.emplace(name, std::move(Vert{name, id}));
	
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

	for (auto& e : v.fanin) {
		// remove edge pointer mapping and recycle free id
		_eptrs[e->id] = nullptr;
		_idxgen_edge.recycle(e->id);
		e->from.remove_fanout(*e);
		assert(e->satellite);
		_edges.erase(*e->satellite);
		e->satellite.reset();
	}
	for (auto& e : v.fanout) {
		_eptrs[e->id] = nullptr;
		_idxgen_edge.recycle(e->id);
		e->to.remove_fanin(*e);
		assert(e->satellite);
		_edges.erase(*e->satellite);
		e->satellite.reset();
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

	std::vector<std::optional<float>> ws = {
		w0, w1, w2, w3, w4, w5, w6, w7
	};

	
	Vert& v_from = insert_vertex(from);
	Vert& v_to = insert_vertex(to);
	
	

	// NOTE: if I use std::pair as key
	// the complier requires a custom hash function
	// which I don't think we're able to define a high-quality hash function?
	// so I simply scan thru the edge vector here
	auto itr = std::find_if(_edges.begin(), _edges.end(), [&](const Edge& e) {
		return (e.from.name == from) && (e.to.name == to);
	});


	if (itr != _edges.end()) {
		// edge exists
		// update weights
		for (size_t i = 0; i < itr->weights.size(); i++) {
			if (!itr->weights[i].has_value()) {
				itr->weights[i] = ws[i];
			}
		}

		return (*itr);
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
		return e; 
	}


	// TODO: study emplace
	//	auto e = _edges.emplace(
	//		std::piecewise_construct,
	//		std::forward_as_tuple(from, to),
  //    std::forward_as_tuple(from_v, to_v, _edges.size(), std::move(vec))
	//	);

}

void Ink::remove_edge(const std::string& from, const std::string& to) {
  auto itr = std::find_if(_edges.begin(), _edges.end(), [&](const Edge& e) {
		return e.from.name == from && e.to.name == to;
	});

	if (itr == _edges.end()) {
		// edge non existent, nothing to do
		return;
	}
	
	_remove_edge(*itr);
}


void Ink::report(size_t K) {

	PathHeap heap;
	auto& v = _name2v["out"];
	Point p(v, 0.0);

	_spur(p, K, heap);
	
}

void Ink::dump(std::ostream& os) const {
	os << num_verts() << " Vertices:\n";
	os << "------------------\n";
	for (const auto& [name, v] : _name2v) {
		os << "name=" << v.name 
			 << ", id=" << v.id << '\n'; 
		os << "... fanins=";
		for (const auto& e : v.fanin) {
			os << e->from.name << ' ';
		}
		os << '\n';
	
		os << "... fanouts=";
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

void Ink::_read_graph(std::istream& is) {
	std::string buf;
	while (true) {
		is >> buf;
		if (is.eof()) {
			break;
		}

		// 1st line: 
		// [n verts] [n edges]
		auto n_verts = std::stoi(buf);
		is >> buf;
		auto n_edges = std::stoi(buf);
		
		// next n_verts lines are vertex names
		for (int i = 0; i < n_verts; i++) {
			is >> buf;
			insert_vertex(buf);
		}

		// next n_edges lines are edge definitions
		// format: [from] [to] [weights] [edge name]
		std::vector<std::optional<float>> ws;
		for (int i = 0; i < n_edges; i++) {
			ws.clear();
			
			is >> buf;
			auto from = buf;
			is >> buf;
			auto to = buf;
			
			for (int j = 0; j < 8; j++) {
				is >> buf;
				if (buf == "n/a") {
					ws.emplace_back(std::nullopt);
				}
				else {
					ws.emplace_back(std::stof(buf));
				}
			}

			insert_edge(from, to, 
				ws[0], ws[1], ws[2], ws[3],
				ws[4], ws[5], ws[6], ws[7]);
			
		}
	
	}
}

Edge& Ink::_insert_edge(
	Vert& from, 
	Vert& to, 
	std::vector<std::optional<float>>&& ws) {
	
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
	_topologize(sfxt, sfxt.T);

	assert(!sfxt.topo_order.empty());

	/// visit the topological order in reverse
	for (auto itr = sfxt.topo_order.rbegin(); 
		itr != sfxt.topo_order.rend(); 
		++itr) {
		auto v = *itr;
		auto pin = _vptrs[v]; 
		assert(pin != nullptr);	

		if (pin->is_src()) {
			sfxt.srcs.try_emplace(v, std::nullopt);
			continue;
		}

		for (auto e : pin->fanin) {
			// relaxation
			sfxt.relax(e->from.id, v, e->id, e->min_valid_weight());	
		
		}

	}	

}

Ink::Sfxt Ink::_sfxt_cache(const Point& p) const {
	// NOTE: OpenTimer: same pin has 2 different configs
	// so it encodes the super src to be 2*[num_v]
	// We can simply use num_v?
	auto S = _vptrs.size();
	auto to = p.vert.id;

	Sfxt sfxt(S, to);

	assert(!sfxt.dists[to]);

	// NOTE: is it correct to initialize root dist to 0?
	sfxt.dists[to] = 0.0;
	
	// calculate shortest path tree with dynamic programming
	_build_sfxt(sfxt);

	// relax from super source to sources
	for (auto& [src, w] : sfxt.srcs) {

		// NOTE: question ...
		// distance from S to srcs should be 0?
		//																v
		w = 0.0;
		sfxt.relax(S, src, std::nullopt, *w);
	}

	
	return sfxt;
}


Ink::Pfxt Ink::_pfxt_cache(const Sfxt& sfxt) const {
	Pfxt pfxt(sfxt);
	assert(sfxt.dist());

	// generate path prefix from each source vertex
	for (const auto& [src, w] : sfxt.srcs) {
		if (!w) {
			continue;
		}
	
		auto src_w = *sfxt.dists[src] + *w;
		pfxt.push(src_w, sfxt.S, src, nullptr, nullptr);
	}
	return pfxt;
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
		if (heap.size() >= K && heap.top()->weight <= node->weight) {
			break;
		}

		// push the path to the path heap
		auto path = std::make_unique<Path>(node->weight, &endpt);

		// recover the complete path

		
	}

} 

void Ink::_recover_path(
	Path& path,
	const Sfxt& sfxt,
	const PfxtNode* pfxt_node,
	size_t v) {

	if (pfxt_node == nullptr) {
		return;
	}

	// recurse until we reach the source pfxt node
	_recover_path(path, sfxt, pfxt_node->parent, pfxt_node->from);

	auto u = pfxt_node->to;
	auto u_vptr = _vptrs[u];

	// if we're at the sfxt source
	// construct a point with distance 0.0
	if (pfxt_node->from == sfxt.S) {
		path.emplace_back(*u_vptr, 0.0);
	}
	// detour from non-sfxt-source nodes
	else {
		assert(!path.empty());

		auto d = path.back().dist + pfxt_node->edge->min_valid_weight();
		path.emplace_back(*u_vptr, d);
	}
	
	// TODO: iterate to end point 
	while (u != v) {
	
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
	float w, 
	size_t f, 
	size_t t, 
	const Edge* e,
	const PfxtNode* p) :
	weight{w},
	from{f},
	to{t},
	edge{e},
	parent{p}
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
	const PfxtNode* p) {
	nodes.emplace_back(std::make_unique<PfxtNode>(w, f, t, e, p));
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


} // end of namespace ink 

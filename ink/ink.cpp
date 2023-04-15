#include "ink.hpp"


namespace ink {


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
int a = 5;
int* ptr = &a;
free(ptr);
}
*/

/// TODO: study what unique_ptr is
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


// TODO: super target and super source are just concept for implementing
// the algorithm. they do not really exist 


void Ink::build_sfxt() {
	//topologize(_sfxt.T);


	/// visit the topological order in reverse
	//or (auto itr = _sfxt.topo_order.rbegin(); 
	// 	 itr != _sfxt.topo_order.rend(); 
	// 	 ++itr) {
	//
	// // TODO: rewrite with linear ordering

	//
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
	for (auto& e : _eptrs) {
		if (e != nullptr) {
			os << "ptr to " << e->from.name << "->" << e->to.name << '\n';
		}
		else {
			os << "ptr to null\n";
		}
	}

	//os << "------------------\n";
	//os << "Topological Order:\n";
	//os << "------------------\n";
	//for (const auto& t : _sfxt.topo_order) {
	//	os << t << ' ';
	//}
	//os << '\n';

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




void Ink::_topologize(const size_t root) {	
	// set visited to true
	//_sfxt.visited[root] = true;

	
	// TODO: rewrite with linear indexing


}




} // end of namespace ink 

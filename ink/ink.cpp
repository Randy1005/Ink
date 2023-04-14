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


void Vert::insert_fanin(Edge& edge) {
	assert(&edge.to == this);
	fanin.emplace_back(std::make_unique<Edge>(edge));
	edge.fanin_satellite = fanin.size() - 1;
}

void Vert::insert_fanout(Edge& edge) {
	assert(&edge.from == this);
	fanout.emplace_back(std::make_unique<Edge>(edge));
	edge.fanout_satellite = fanout.size() - 1;
}

void Vert::remove_fanin(Edge& edge) {
	//assert(edge.fanin_satellite);
	//assert(&edge.to == this);
	//
	//const auto sat = edge.fanin_satellite.value();
	//// swap and pop method
	//fanin[sat] = fanin.back();
	//fanin.pop_back();

	//// the final element has been relocated, update its satellite index
	//fanin[sat]->fanin_satellite = sat;
}

void Vert::remove_fanout(Edge& edge) {
	//assert(edge.fanout_satellite);
	//assert(&edge.from == this);
	//
	//const auto sat = edge.fanout_satellite.value();
	//// swap and pop method
	//fanout[sat] = fanout.back();
	//fanout.pop_back();

	//// the final element has been relocated, update its satellite index
	//fanout[sat]->fanout_satellite = sat;
}

void Ink::read_graph(const std::string& file) {
	std::ifstream ifs;
	ifs.open(file);
	if (!ifs) {
		throw std::runtime_error("Failed to open file: " + file);
	}

	_read_graph(ifs);
}


void Ink::insert_vertex(const std::string& name) {
	const auto id = _verts.size();
	_verts.insert({ name, std::move(Vert{name, id}) });
}

void Ink::remove_vertex(const std::string& name) {
	auto found = _verts.find(name);
	if (found != _verts.end()) {
		// remove all fanout, fanin edges of this vertex
		auto& v = (*found).second;

		for (auto& e : v.fanin) {
			v.remove_fanin(*e);
			(*e).from.remove_fanout(*e);
			
			// update the main edge storage
			remove_edge((*e).from.name, (*e).to.name);
		}
		
		for (auto& e : v.fanout) {
			v.remove_fanout(*e);
			(*e).to.remove_fanin(*e);
		
			// update the main edge storage
			remove_edge((*e).from.name, (*e).to.name);
		
	
			// invalidate optional fanin, fanout satellites
			(*e).fanin_satellite.reset();
			(*e).fanout_satellite.reset();
		}

		// TODO:
		// I'm removing edges from "_edges"
		// by scanning the whole storage 
		// finding matches (API), can be done more
		// efficiently with another satellite index

		_verts.erase(found);
	}
	else {
		std::cerr << "Attempt to remove an non-existent vertex\n";
	}

	
}

void Ink::insert_edge(
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
	
	const auto id = _edges.size();
	
	// create vertices if any of the input vertices doesn't exist
	insert_vertex(from);
	insert_vertex(to);

	std::vector<std::optional<float>> weights = {
		w0, w1, w2, w3, w4, w5, w6, w7
	};

	Edge e(_verts[from], _verts[to], id, std::move(weights));

	e.from.insert_fanout(e);
	e.to.insert_fanin(e);
	
	_edges.push_back(std::move(e));
}

void Ink::remove_edge(const std::string& from, const std::string& to) {
//	for (size_t i = 0; i < _edges.size(); i++) {
//		if (_edges[i].from.name == from && _edges[i].to.name == to) {
//			_edges_swap_and_pop(i);	
//		}
//	}
}


void Ink::create_super_src(const std::string& src_name) {
	insert_vertex(src_name);
	_sfxt._S = src_name;
	for (const auto& [name, v] : _verts) {
		if (v.is_src() && name != src_name) {
			insert_edge(
				src_name, name, 
				0, 0, 0, 0, 
				0, 0, 0, 0);
		}
	}
}

void Ink::create_super_dst(const std::string& dst_name) {
	insert_vertex(dst_name);
	_sfxt._T = dst_name;
	for (const auto& [name, v] : _verts) {
		if (v.is_dst() && name != dst_name) {
			insert_edge(
				name, dst_name, 
				0, 0, 0, 0, 
				0, 0, 0, 0);
		}
	}
}

void Ink::build_sfxt() {
	_topologize(_sfxt._T);

	// visit the topological order in reverse
	for (auto itr = _sfxt._topo_order.rbegin(); 
			 itr != _sfxt._topo_order.rend(); 
			 ++itr) {
		
		auto v = *itr;

	}
}

void Ink::dump(std::ostream& os) const {
	os << num_verts() << " Vertices:\n";
	os << "------------------\n";
	for (const auto& [name, v] : _verts) {
		os << "name=" << v.name 
			 << ", id=" << v.id << '\n'; 
		os << "... fanins=";
		for (const auto& e : v.fanin) {
			os << (*e).from.name << ' ';
		}
		os << '\n';
	
		os << "... fanouts=";
		for (const auto& e : v.fanout) {
			os << (*e).to.name << ' ';
		}
		os << '\n';

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
	os << "Topological Order:\n";
	os << "------------------\n";
	for (const auto& t : _sfxt._topo_order) {
		os << t << ' ';
	}
	os << '\n';
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

void Ink::_topologize(const std::string& root) {	
	// set visited to true
	_sfxt._visited[root] = true;

	// base case:
	// stop at path source
	const auto& v = _verts[root];
	if (!v.is_src()) {
		for (const auto& e : v.fanin) {
			const auto& neighbor = (*e).from;
			if (!_sfxt._visited[neighbor.name]) {
				_topologize(neighbor.name);
			}
		}
	}


	_sfxt._topo_order.emplace_back(root);	

}




} // end of namespace ink 

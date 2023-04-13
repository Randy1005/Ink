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
	_verts.emplace(name, std::move(Vert{name, id}));
}

void Ink::remove_vertex(const std::string& name) {
	auto found = _verts.find(name);
	if (found != _verts.end()) {
		// remove all fanout, fanin edges of this vertex
		
		auto& v = (*found).second;
		for (const auto& in : v.fanins) {
			// update the fanout edges for all from vertices
			_verts[in].fanouts_swap_and_pop(v.name);

			// remove the fanin edges
			remove_edge(in, v.name);
		}	
		
		for (const auto& out : v.fanouts) {
			// update the fanin edges for all to vertices
			_verts[out].fanins_swap_and_pop(v.name);

			// remove the fanout edges
			remove_edge(v.name, out);
		}

		_verts.erase(found);
	}
	else {
		std::cout << "Attempt to remove an non-existent vertex\n";
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

	// populate fanins, fanouts 
	auto& v_from = _verts[from];
	auto& v_to = _verts[to];
	
	v_from.fanouts.emplace_back(to);
	v_to.fanins.emplace_back(from);
	
	std::vector<std::optional<float>> weights = {
		w0, w1, w2, w3, w4, w5, w6, w7
	};

	_edges.emplace_back(v_from, v_to, id, std::move(weights));

	
}

void Ink::remove_edge(const std::string& from, const std::string& to) {
	for (size_t i = 0; i < _edges.size(); i++) {
		if (_edges[i].from.get().name == from && _edges[i].to.get().name == to) {
			_edges_swap_and_pop(i);	
		}
	}

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


}

void Ink::dump(std::ostream& os) const {
	os << num_verts() << " Vertices:\n";
	os << "------------------\n";
	for (const auto& v : _verts) {
		os << "name=" << v.second.name 
			 << ", id=" << v.second.id 
			 << ", num fanins=" << v.second.fanins.size()
			 << ", num fanouts=" << v.second.fanouts.size() << '\n';
	}

	os << "------------------\n";
	os << num_edges() << " Edges:\n";
	os << "------------------\n";
	for (const auto& e : _edges) {
		os << "id=" << e.id
			 << " ... name= " << e.from.get().name 
			 << "->" << e.to.get().name
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
		for (const auto& neighbor : v.fanins) {
			if (!_sfxt._visited[neighbor]) {
				_topologize(neighbor);
			}
		}
	}

	_sfxt._topo_order.emplace_back(root);
}




} // end of namespace ink 

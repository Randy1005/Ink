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
void Vert::insert_fanin(Edge& edge) {
}

void Vert::insert_fanout(Edge& edge) {
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
	
		// if the index generated goes out of range
		// resize _vptrs 
		if (id + 1 > _vptrs.size()) {
			_vptrs.resize(id + 1);
		}

		_vptrs[id] = &(iter->second); 
		return iter->second;	
	}
	else {
		return itr->second;
	}
	
	
}

void Ink::remove_vertex(const std::string& name) {

	
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
	}
	else {

		// edge doesn't exist
		// check if there's a free id to assign
		if (!_efree.empty()) {
			auto id = _efree.back();
			_efree.pop_back();

			// insert this new edge into the free slot
			//auto beg = _edges.begin();
			//auto itr = edges.insert(std::next(beg, id), {v_from, v_to, id, std::move(ws)});
			
		}
		else {
			// no free id to assign, need to increase list size
			auto& e = _edges.emplace_back(v_from, v_to, _edges.size(), std::move(ws));
		
			e.from.fanout.push_back(&e);	
			e.to.fanin.push_back(&e);	
		}

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
	
	auto& v_from = itr->from;
	auto& v_to = itr->to;

	// add this edge id to edge free list
	_efree.push_back(itr->id);

	/// TODO:
	//auto itr = _edges.find({from, to});
	//if(itr == _edges.end()) {
	//  return;
	//}
	////
	//auto& from_v = itr->second.from;
	//auto& to_v   = itr->second.to;
	//from_v.fanout.remove(&(itr->second));
	//to_v.fanin.revmoe(&(itr->second));
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

void Ink::_topologize(const size_t root) {	
	// set visited to true
	//_sfxt.visited[root] = true;

	
	// TODO: rewrite with linear indexing


}




} // end of namespace ink 

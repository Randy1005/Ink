#include "ink.hpp"


namespace ink {

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
	// remove the vertex with key "name"
	// from unordered map
	auto found = _verts.find(name);
	if (found != _verts.end()) {
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


	std::vector<std::optional<float>> weights = {
		w0, w1, w2, w3, w4, w5, w6, w7
	};

	_edges.emplace_back(_verts[from], _verts[to], id, std::move(weights));
}

void Ink::dump(std::ostream& os) const {
	os << num_verts() << " Vertices:\n";
	os << "------------------\n";
	for (const auto& v : _verts) {
		os << v.first << ": name="
							<< v.second.name << ", id="
							<< v.second.id << '\n';
	}

	os << "------------------\n";
	os << num_edges() << " Edges:\n";
	os << "------------------\n";
	for (const auto& e : _edges) {
		os << "name: " << e.from.name 
			 << "->" << e.to.name
			 << " ... weights = ";
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





} // end of namespace ink 

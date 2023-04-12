#include "ink.hpp"


namespace ink {

void Ink::insert_vertex(const std::string& name) {
	const auto id = _verts.size();
	_verts.emplace(name, std::move(Vert{name, id}));
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

	// we enforce the edge name to be:
	// [from_vertex]->[to_vertex]
	// for the ease of lookup
	const std::string edge_name = from + "->" + to; 
	
	std::vector<std::optional<float>> weights = {
		w0, w1, w2, w3, w4, w5, w6, w7
	};

	_edges.emplace(
		edge_name,
		std::move(
			Edge(_verts[from], _verts[to], edge_name, id, std::move(weights))
		)
	);
}

void Ink::dump(std::ostream& os) const {
	std::cout << "Vertices:\n";
	std::cout << "------------------\n";
	for (const auto& v : _verts) {
		std::cout << v.first << ": name="
							<< v.second.name << ", id="
							<< v.second.id << '\n';
	}

	std::cout << "------------------\n";
	std::cout << "Edges:\n";
	std::cout << "------------------\n";
	for (const auto& e : _edges) {
		std::cout << "name: " << e.first << " ... From "
							<< e.second.from.name << " to "
							<< e.second.to.name << " ... weights = ";
		for (const auto& w : e.second.weights) {
			if (w.has_value()) {
				std::cout << w.value() << ' ';
			}
			else {
				std::cout << "n/a ";
			}
		}
		std::cout << '\n';
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
	}



}





} // end of namespace ink -------------------

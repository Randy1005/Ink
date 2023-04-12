#pragma once
#include <ot/timer/timer.hpp>
#include <iostream>
#include <string>

namespace ink {


struct Vert;
struct Edge;

struct Vert {
	Vert() = default;
	Vert(const std::string& name, const size_t id) :
		name{name},
		id{id}
	{
	}

	Vert(Vert&&) = default;
	Vert(const Vert&) = default;
	
	std::string name;
	size_t id;
};

struct Edge {
	Edge(Vert& v_from, Vert& v_to, 
			 const std::string& name, 
			 const size_t id,
			 const std::vector<std::optional<float>>& ws) :
		from{v_from},
		to{v_to},
		id{id},
		weights{ws}
	{
	}

	Edge(Edge&&) = default;
	Edge(const Edge&) = default;

	Vert& from;
	Vert& to;
	size_t id;
	std::string name;

	// vector of optional weights
	std::vector<std::optional<float>> weights;	
};




class Ink {
public:
	void read_graph(const std::string& file);
	void insert_vertex(const std::string& name);
	void insert_edge(
		const std::string& from,
		const std::string& to,
		const std::optional<float> w0,
		const std::optional<float> w1,
		const std::optional<float> w2,
		const std::optional<float> w3,
		const std::optional<float> w4,
		const std::optional<float> w5,
		const std::optional<float> w6,
		const std::optional<float> w7);

	void dump(std::ostream& os) const;
private:

	void _read_graph(std::istream& is);
	// vector of vertices
	std::unordered_map<std::string, Vert> _verts;
	
	// vector of edges
	std::unordered_map<std::string, Edge> _edges;


};









} // end of namespace inkt -------------------


#pragma once
#include <ot/timer/timer.hpp>
#include <iostream>
#include <string>

namespace ink {

struct Vert;
struct Edge;
class Ink;

struct Vert {
	Vert() = default;
	Vert(const std::string& name, const size_t id) :
		name{name},
		id{id}
	{
	}

	Vert(Vert&&) = default;
	
	std::string name;
	size_t id;
	std::vector<Edge*> fanins;
	std::vector<Edge*> fanouts;
};

struct Edge {
	Edge(Vert& v_from, Vert& v_to, 
			 const size_t id,
			 const std::vector<std::optional<float>>& ws) :
		from{v_from},
		to{v_to},
		id{id},
		weights{ws}
	{
	}

	Edge(Edge&&) = default;

	Vert& from;
	Vert& to;
	size_t id;

	// vector of optional weights
	std::vector<std::optional<float>> weights;	
};




class Ink {
public:
	void read_graph(const std::string& file);
	void insert_vertex(const std::string& name);
	void remove_vertex(const std::string& name);
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

	// NOTE:
	// I think update edge has to be by index?
	// because based on simple.cpp, multiple edges can have
	// the same name (even same set of weights)
	void update_edge(
		size_t id,
		const std::optional<float> w0,
		const std::optional<float> w1,
		const std::optional<float> w2,
		const std::optional<float> w3,
		const std::optional<float> w4,
		const std::optional<float> w5,
		const std::optional<float> w6,
		const std::optional<float> w7);


	void dump(std::ostream& os) const;

	inline size_t num_verts() const {
		return _verts.size();
	} 

	inline size_t num_edges() const {
		return _edges.size();
	}
private:

	void _read_graph(std::istream& is);
	// unordered map of vertices
	std::unordered_map<std::string, Vert> _verts;
	
	// vector of edges
	std::vector<Edge> _edges;


};









} // end of namespace ink


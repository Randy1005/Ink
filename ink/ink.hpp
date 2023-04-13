#pragma once
#include <ot/timer/timer.hpp>
#include <iostream>
#include <string>
#include <algorithm>


namespace ink {

struct Vert;
struct Edge;
class Ink;


struct Edge {
	Edge() = default;
	Edge(Vert& v_from, Vert& v_to, 
			 const size_t id,
			 const std::vector<std::optional<float>>& ws) :
		from{v_from},
		to{v_to},
		id{id},
		weights{ws}
	{
	}

	Edge(const Edge&) = default;

	std::reference_wrapper<Vert> from;
	std::reference_wrapper<Vert> to;
	size_t id;

	// vector of optional weights
	std::vector<std::optional<float>> weights;	
};

struct Vert {

	Vert() = default;
	Vert(const std::string& name, const size_t id) :
		name{name},
		id{id}
	{
	}

	Vert(Vert&&) = default;

	inline void fanins_swap_and_pop(
		const std::string& from) {
		for (size_t i = 0; i < fanins.size(); i++) {
			if (fanins[i] == from) {
				fanins[i] = std::move(fanins.back());
				fanins.pop_back();
			}
		}
	}


	inline void fanouts_swap_and_pop(
		const std::string& to) {
		for (size_t i = 0; i < fanouts.size(); i++) {
			if (fanouts[i] == to) {
				fanouts[i] = std::move(fanouts.back());
				fanouts.pop_back();
			}
		}
	}


	std::string name;
	size_t id;
	std::vector<std::string> fanins;
	std::vector<std::string> fanouts;
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

	void remove_edge(const std::string& from, const std::string& to);

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
	
	inline void _edges_swap_and_pop(size_t i) {
		_edges[i] = std::move(_edges.back());
		_edges.pop_back();
		_edges[i].id = i;
	}
	
	// unordered map of vertices
	std::unordered_map<std::string, Vert> _verts;
	
	// vector of edges
	std::vector<Edge> _edges;


};









} // end of namespace ink


#pragma once
#include <ot/timer/timer.hpp>
#include <iostream>
#include <string>
#include <algorithm>

namespace ink {

struct Vert;
struct Edge;
class Sfxt;
class Ink;



struct Vert {
	Vert() = default;
	Vert(const std::string& name, const size_t id) :
		name{name},
		id{id}
	{
	}

	bool is_src() const;
	bool is_dst() const;

	bool operator == (const Vert& v) const {
		return v.id == id;
	}

	inline size_t num_fanins() const {
		return fanin.size();
	}

	inline size_t num_fanouts() const {
		return fanout.size();
	}

	void insert_fanout(Edge& e);
	void insert_fanin(Edge& e);

	void remove_fanout(Edge& e);
	void remove_fanin(Edge& e);

	std::string name;
	size_t id;

	// TODO: reason why using unique pointer is bad
	// and understand how unique pointer is implemented



	// TODO: best practice - and tell me why the above code is shit
	std::list<Edge*> fanin;
	std::list<Edge*> fanout;
};


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


	bool operator == (const Edge& e) const {
		return e.from.id == from.id && e.to.id == to.id;
	}

	Vert& from;
	Vert& to;
	size_t id;


	std::optional<size_t> fanout_satellite;
	std::optional<size_t> fanin_satellite;

	// vector of optional weights
	std::vector<std::optional<float>> weights;	
};



/**
@brief Suffix tree class
*/

// TODO: move sfxt class to Ink



class Ink {
	
public:
	Ink() = default;	

	void read_graph(const std::string& file);

	// NOTE: should return a reference
	// insert_edge would need it
	Vert& insert_vertex(const std::string& name);
	
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


	void build_sfxt();

	void dump(std::ostream& os) const;

	inline size_t num_verts() const {
		return _name2v.size();
	} 

	inline size_t num_edges() const {
		return _edges.size();
	}


private:
	struct Sfxt {
		Sfxt() = default;

		// super source
		size_t S;

		// suffix tree root
		size_t T;

		// topological order of vertices
		std::vector<size_t> topo_order; 

		// to record if visited in topological sort
		std::vector<bool> visited;

		// distances 
		std::vector<float> dists;
		
		// parents
		std::vector<size_t> parents;
	};
	
	void _read_graph(std::istream& is);
	void _topologize(const size_t root);

	// unordered map: name to vertex object
	// NOTE: this is the owner storage
	// anything that need access to vertices would store a pointer to it
	std::unordered_map<std::string, Vert> _name2v;
	
	std::vector<Vert*> _vptrs;

	// vertices free list
	// NOTE: to reuse deleted vertices' id
	std::vector<size_t> _vfree;   // TODO: study freelist

	std::vector<Edge> _edges;
	
	// edges free list
	// NOTE: to reuse deleted edges' id 
	std::vector<size_t> _efree;
	
	// suffix tree
	Sfxt _sfxt;
};


// NOTE: concept of free list
//  0 , 1,  2,  3,  4
// v1, v2, v3, v4, v5
//
// remove v3  => vfree.push-back 2

//  0 , 1,  2,   3,  4
// v1, v2, nil, v4, v5
//
// insert v6 => v6.id = vfree.pop_back() if any, or vertices.size()
//
//  0 , 1,  2,   3,  4
// v1, v2, v6,  v4, v5

} // end of namespace ink


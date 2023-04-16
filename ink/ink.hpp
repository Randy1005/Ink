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


/**
@brief Vertex
*/
struct Vert {
	Vert() = default;
	Vert(const std::string& name, const size_t id) :
		name{name},
		id{id}
	{
	}

	Vert(Vert&&) = default;
	Vert& operator = (const Vert&) = default; 
	Vert(const Vert&) = default;

	bool is_src() const;
	bool is_dst() const;

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
			 std::vector<std::optional<float>>&& ws) :
		from{v_from},
		to{v_to},
		id{id},
		weights{std::move(ws)}
	{
	}

	std::string name() const;

	// TODO: is this the right weight to use? 
	inline float min_valid_weight() const {
		float v = std::numeric_limits<float>::max();
		for (auto w : weights) {
			if (w.has_value() && w.value() < v) {
				v = w.value();
			}
		}
		return v;
	}


	Vert& from;
	Vert& to;
	size_t id;

	std::optional<std::list<Edge>::iterator> satellite;
	std::optional<std::list<Edge*>::iterator> fanout_satellite;
	std::optional<std::list<Edge*>::iterator> fanin_satellite;


	// vector of optional weights
	std::vector<std::optional<float>> weights;	
};



class Ink {
	
public:
	Ink() = default;	

	void read_graph(const std::string& file);

	Vert& insert_vertex(const std::string& name);
	
	void remove_vertex(const std::string& name);

	Edge& insert_edge(
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

	void report(const size_t k);

	void dump(std::ostream& os) const;

	inline size_t num_verts() const {
		return _name2v.size();
	} 

	inline size_t num_edges() const {
		return _edges.size();
	}


private:

	/**
	@brief Suffix Tree
	*/
	struct Sfxt {
		Sfxt() = default;

		// super source
		size_t S;

		// suffix tree root
		size_t T;

		// sources
		std::unordered_map<size_t, std::optional<float>> srcs;

		// topological order of vertices
		std::vector<size_t> topo_order; 

		// to record if visited in topological sort
		std::vector<bool> visited;

		// distances 
		std::vector<float> dists;
		
		// parents
		std::vector<size_t> parents;
	
		// links (edge ids)
		std::vector<size_t> links;


	};

	/**
	@brief Index Generator
	*/
	struct IdxGen {
		IdxGen() = default;

		inline size_t get() {
			if (freelist.empty()) {
				return counter++;
			}
			
			// pop a free idx from free list
			auto idx = freelist.back();
			freelist.pop_back();
			return idx;
		
		}

		inline void recycle(size_t free_id) {
			// push this id to free list
			freelist.push_back(free_id);
		}

		size_t counter = 0;
		std::vector<size_t> freelist;
	};

	
	void _read_graph(std::istream& is);
	
	void _topologize(const size_t root);

	void _build_sfxt();
	
	Edge& _insert_edge(
		Vert& from, 
		Vert& to,
		std::vector<std::optional<float>>&& ws);
	void _remove_edge(Edge& e);

	// unordered map: name to vertex object
	// NOTE: this is the owner storage
	// anything that need access to vertices would store a pointer to it
	std::unordered_map<std::string, Vert> _name2v;

	// ordered pointer storage
	std::vector<Vert*> _vptrs;

	std::list<Edge> _edges;
	
	std::vector<Edge*> _eptrs;

	// suffix tree
	Sfxt _sfxt;

	// index generator : vertices
	// NOTE: free list is defined in this object
	IdxGen _idxgen_vert;


	// index generator : edges
	// NOTE: free list is defined in this object
	IdxGen _idxgen_edge;
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


#pragma once
#include <ot/timer/timer.hpp>

namespace ink {

struct Vert;
struct Edge;
struct Sfxt;
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
	@brief Prefix Tree Node
	*/
	struct PfxtNode {
		PfxtNode(float w, size_t f, size_t t, const Edge* e, const PfxtNode* p);

		float weight;
		size_t from;
		size_t to;
		const Edge* edge{nullptr};
		const PfxtNode* parent{nullptr}; 
	};


	/**
	@brief Prefix Tree
	*/
	struct Pfxt {
		struct PfxtNodeComp {
			bool operator() (
				const std::unique_ptr<PfxtNode>& a,
				const std::unique_ptr<PfxtNode>& b) const {
				return a->weight > b->weight;
			}	
		};

		Pfxt(const Sfxt& sfxt);
		Pfxt(Pfxt&& other);
		Pfxt& operator = (Pfxt&& other) = delete;

		inline size_t num_nodes() const {
			return nodes.size();
		} 
		
		void push(
			float w,
			size_t f,
			size_t t,
			const Edge* e,
			const PfxtNode* p);		
		
		PfxtNode* pop();

		const Sfxt& sfxt;
		
		// prefix node comparator object
		PfxtNodeComp comp;

		// path
		std::vector<std::unique_ptr<PfxtNode>> paths;
		
		// nodes (to use as a min heap)
		std::vector<std::unique_ptr<PfxtNode>> nodes;
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


} // end of namespace ink


#pragma once
#include <stack>
#include <ot/timer/timer.hpp>
#include <ot/taskflow/algorithm/reduce.hpp>

#define NUM_WEIGHTS 8

namespace ink {

struct Vert;
struct Edge;
struct Respur;
struct Sfxt;
struct PfxtNode;
struct Pfxt;
class Ink;
struct Point;
struct Path;
class PathHeap;


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
	std::list<Edge*> fanin;
	std::list<Edge*> fanout;

	// to record whether this vertex 
	// is already marked as to be updated
	// we don't want duplicates in the update list
	bool is_in_update_list{false};
};


struct Edge {
	Edge() = default;
	Edge(Vert& from, Vert& to, 
			 const size_t id,
			 std::array<std::optional<float>, 8>&& ws) :
		from{from},
		to{to},
		id{id},
		weights{std::move(ws)}
	{
	}

	std::string name() const;

	/**
	@brief returns the index of the minimum weight
	(excluding std::nullopts)
	*/
	inline size_t min_valid_weight() const;


	Vert& from;
	Vert& to;
	size_t id;

	// satellite iterator indicating the position of this edge in edge list
	std::optional<std::list<Edge>::iterator> satellite;

	// satellite iterator indicating the position of this edge 
	// in a vertex's fanout list
	std::optional<std::list<Edge*>::iterator> fanout_satellite;

	// satellite iterator indicating the position of this edge 
	// in a vertex's fanin list
	std::optional<std::list<Edge*>::iterator> fanin_satellite;
	
	// static array of optional weights
	std::array<std::optional<float>, NUM_WEIGHTS> weights;

	// records if an edge's weights had been modified
	bool modified{false};

	// records which pfxt nodes depend on this edge
	std::vector<PfxtNode*> dep_nodes;
	
	// records which weights are pruned on this edge
	std::array<bool, NUM_WEIGHTS> pruned_weights;
};

/**
@brief Re-Spur List Element
*/
struct Respur {
	Respur(
		PfxtNode* node, 
		const std::vector<std::pair<size_t, size_t>>&& pruned) :
		node{node},
		pruned{std::move(pruned)}
	{
	}


	PfxtNode* node;
	std::vector<std::pair<size_t, size_t>> pruned;
};


/**
@brief Suffix Tree
*/
struct Sfxt {
	Sfxt() = default;
	Sfxt(size_t S, size_t T, size_t sz);

	inline bool relax(
		size_t u, 
		size_t v, 
		std::optional<std::pair<size_t, size_t>> l,
		float d);
	
	/**
	@brief returns the distance from super source to 
	suffix tree root
	*/
	inline std::optional<float> dist() const;

	// super source
	size_t S;

	// suffix tree root
	size_t T;

	// sources
	std::unordered_map<size_t, std::optional<float>> srcs;

	// topological order of vertices
	std::vector<size_t> topo_order;

	// to record if visited in topological sort
	std::vector<std::optional<bool>> visited;

	// distances 
	std::vector<std::optional<float>> dists;
	
	// successors
	std::vector<std::optional<size_t>> successors;

	// links (edge id, weight selection) 
	std::vector<std::optional<std::pair<size_t, size_t>>> links;
};




/**
@brief Prefix Tree Node
*/
struct PfxtNode {
	PfxtNode(
		float c, 
		size_t f, 
		size_t t, 
		Edge* e, 
		PfxtNode* p,
		std::optional<std::pair<size_t, size_t>> l);
	
	float cost;
	size_t from;
	size_t to;
	Edge* edge{nullptr};
	PfxtNode* parent{nullptr};
	std::optional<std::pair<size_t, size_t>> link;

	size_t num_children() const;

	// for traversing the prefix tree
	std::vector<PfxtNode*> children;

	// edge id + weight selection pairs to prune for re-spur
	std::vector<std::pair<size_t, size_t>> pruned;

	// record this node's subtree sequence index
	// begin and end
	size_t subtree_beg;
	size_t subtree_end;

	// record which level this pfxt node is located
	// in the prefix tree
	size_t level;

	// record if visited by a leader's upward traversal
	bool visited_by_leader{false};

	bool updated{false};
	bool removed{false};
	bool to_respur{false};
};


/**
@brief Prefix Tree
*/
struct Pfxt {
	struct PfxtNodeComp {
		bool operator() (
			const std::unique_ptr<PfxtNode>& a,
			const std::unique_ptr<PfxtNode>& b) const;
	};

	Pfxt(const Sfxt& sfxt);
	Pfxt(Pfxt&& other);
	Pfxt& operator = (Pfxt&& other) = delete;

	inline size_t num_nodes() const {
		return nodes.size();
	} 
	
	PfxtNode* push(
		float c,
		size_t f,
		size_t t,
		Edge* e,
		PfxtNode* p,
		std::optional<std::pair<size_t, size_t>> l);
	
	PfxtNode* pop();
	
	PfxtNode* top() const;	

	const Sfxt& sfxt;
	
	// prefix node comparator object
	PfxtNodeComp comp;

	// path nodes (basically the nodes that have been
	// popped from the heap)
	std::vector<std::unique_ptr<PfxtNode>> paths;

	// nodes (to use as a min heap)
	std::vector<std::unique_ptr<PfxtNode>> nodes;

	// sources
	std::vector<PfxtNode*> srcs;

};



class Ink {
	
public:
	Ink() = default;	

	void read_ops(
		const std::string& in, 
		const std::string& out);

	Vert& insert_vertex(const std::string& name);

	void remove_vertex(const std::string& name);

	Edge& insert_edge(
		const std::string& from,
		const std::string& to,
		const std::optional<float> w0 = std::nullopt,
		const std::optional<float> w1 = std::nullopt,
		const std::optional<float> w2 = std::nullopt,
		const std::optional<float> w3 = std::nullopt,
		const std::optional<float> w4 = std::nullopt,
		const std::optional<float> w5 = std::nullopt,
		const std::optional<float> w6 = std::nullopt,
		const std::optional<float> w7 = std::nullopt);

	Edge& get_edge(const std::string& from, const std::string& to);

	void remove_edge(const std::string& from, const std::string& to);

	std::vector<Path> report(size_t K);

	std::vector<Path> report_incsfxt(
		size_t K, 
		bool save_pfxt_nodes = false,
		bool recover_paths = true);
	std::vector<Path> report_rebuild(size_t K);
	std::vector<Path> report_incremental(
		size_t K, 
		bool save_pfxt_nodes = false,
		bool use_leaders = false,
		bool recover_paths = true);
	

	void dump(std::ostream& os) const;

	void dump_pfxt_srcs(std::ostream& os) const;

	void dump_pfxt_nodes(std::ostream& os) const;

	void dump_profile(std::ostream& os, bool reset = false);

	/**
	@brief randomly pick a vertex and modify its fanin / fanouts
	(for the purpose of measuring difference between queries)
	*/
	void modify_random_vertex();

	/**
	@brief outputs the percentage of difference between 2 path weight vectors
	*/
	float vec_diff(
		const std::vector<Path>& ps1, 
		const std::vector<Path>& ps2,
		std::vector<float>& diff);

	/**
	@brief get the path costs
	*/
	std::vector<float> get_path_costs();

	inline size_t num_verts() const {
		return _name2v.size();
	} 

	inline size_t num_edges() const {
		return _edges.size();
	}

	// records if a link (edge, w_sel pair) became the sfxt's link
	std::vector<std::array<bool, NUM_WEIGHTS>> belongs_to_sfxt;

private:
	/**
	@brief Index Generator
	*/
	struct IdxGen {
		IdxGen(size_t c_init) :
			counter{c_init}
		{
		}

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

	
	void _read_ops(std::istream& is, std::ostream& os);
	
	void _topologize(Sfxt& sfxt, size_t root) const;

	void _build_sfxt(Sfxt& sfxt) const;

	void _update_sfxt(Sfxt& sfxt) const;

	/**
	@brief Find the suffix tree rooted at super target
	NOTE: a global suffix tree
	*/
	void _sfxt_cache();
	void _sfxt_rebuild();


	/**
	@brief Find the suffix tree rooted at the vertex
	*/
	Sfxt _sfxt_cache(const Point& p) const;
	
	/**
	@brief Spur from the path. Scan the current critical path
	and spur along the path to generate other candidates
	NOTE: using a global suffix tree
	*/
	void _spur_incsfxt(
		size_t K, 
		PathHeap& heap, 
		Pfxt& pfxt,
		bool save_pfxt_nodes = false,
		bool recover_paths = true);

	void _spur_rebuild(size_t K, PathHeap& heap);
	
	void _spur_incremental(
		size_t K, 
		Pfxt& pfxt,
		bool save_pfxt_nodes = false,
		bool use_leaders = false,
		bool recover_paths = true);

	/**
	@brief Spur from the path. Scan the current critical path
	and spur along the path to generate other candidates
	*/
	void _spur(Point& endpt, size_t K, PathHeap& heap);


	/**
	@brief Spur along the path to generate more pfxt node children, 
	given a prefix node, until we reach the suffix tree's super target
	*/
	void _spur(Pfxt& pfxt, PfxtNode& pfx);
	
	/**
	@brief Spur along the path to generate more pfxt node children, 
	given a prefix node, until we reach the suffix tree's super target
	(with a set of pruned edges)
	*/
	void _spur_pruned(
		Pfxt& pfxt, 
		PfxtNode& pfx, 
		const std::vector<std::pair<size_t, size_t>>& pruned);
	

	/**
	@brief Construct a prefixt tree from a given suffix tree
	*/
	Pfxt _pfxt_cache(const Sfxt& sfxt) const;

	/**
	@brief apply euler tour to identify leader prefix tree nodes, 
	i.e. nodes that lead a subtree which is not affected and reusable
	(also identify the subtree sequence in the dfs traversal)
	*/
	void _identify_leaders(
		PfxtNode* curr,
		std::vector<PfxtNode*>& dfs_full,
		std::vector<PfxtNode*>& dfs_marked,
		size_t& order);

	/**
	@brief apply upwared prefix tree traversal to each modified node
	and identifies the subtrees that are not affected
	*/
	void _identify_leaders_upward();

	/**
	@brief update pfxt nodes and markings
	1. updated nodes: propagate costs to its subtree 
		until it reaches a removed node
	2. removed nodes: add its parent to the "re-spur" list
		mark its left hand side siblings as "pruned", to prevent duplicate spur,
		and mark its right hand side siblings as "removed"
	*/
	void _update_pfxt(Pfxt& pfxt);

	/**
	@brief propagate costs from the leader downwards to the subtree it leads
	@param leader the subtree root from the old pfxt
	@param node the pfxt node in the new pfxt
	@param the new pfxt we're currently expanding
	*/
	void _propagate_subtree(
		PfxtNode* leader, 
		PfxtNode* node,
		Pfxt& pfxt);


	/**
	@brief Recover the complete path from a given prefix tree node
	w.r.t a suffix tree
	*/
	void _recover_path(
		Path& path, 
		const Sfxt& sfxt,
		const PfxtNode* pfxt_node,
		size_t v) const;

	void _recover_path(Path& path, const Sfxt& sfxt) const;

	Edge& _insert_edge(
		Vert& from, 
		Vert& to,
		std::array<std::optional<float>, 8>&& ws);
	
	void _remove_edge(Edge& e);

	/**
	@brief encode an edge with different weight selections
	into distinct ids, rule as follows:
		edge id = n, weight index = k
		encoded id = k * num_edges + n

		e.g. num_edges = 10
		edge id = 7, weight index = 0
		encode_edge = 0 * 10 + 7 = 7

		edge id = 7, weight index = 2
		encode_edge = 2 * 10 + 7 = 27

		... and so on
	*/
	inline auto _encode_edge(const Edge& e, size_t w_sel) const {
		// return w_sel * _eptrs.size() + e.id;  	
		return std::make_pair(e.id, w_sel);
	}

	/**
	@brief returns a tuple {ptr_to_edge, weight_idx} 
	*/
	inline auto _decode_edge(size_t idx) const {
		return std::make_tuple(_eptrs[idx % _eptrs.size()], idx / _eptrs.size());
	}

	/**
	@brief returns a tuple {ptr_to_edge, weight_idx} 
	*/
	inline auto _decode_edge(const std::pair<size_t, size_t>& p) const {
		return std::make_tuple(_eptrs[p.first], p.second);
	}

	/**
	@brief clears the update list for next incremental action
	*/
	void _clear_update_list();

	/**
	@brief depth first search to generate topological order
	*/
	void _dfs(
		size_t v, 
		std::deque<size_t>& tpg, 
		std::vector<bool>& visited);


	std::vector<Path> _extract_paths(std::vector<std::unique_ptr<Path>>& paths);

	
	std::vector<Point> _endpoints;

	// unordered map: name to vertex object
	// NOTE: this is the owner storage
	// anything that need access to vertices would store a pointer to it
	std::unordered_map<std::string, Vert> _name2v;

	// unordered map:
	// mapping from edge name to std::list<Edge>::iterator
	std::unordered_map<std::string, std::list<Edge>::iterator> _name2eit;
	
	// ordered pointer storage
	std::vector<Vert*> _vptrs;

	std::list<Edge> _edges;
	
	std::vector<Edge*> _eptrs;

	// index generator : vertices
	// NOTE: free list is defined in this object
	IdxGen _idxgen_vert{2};

	// index generator : edges
	IdxGen _idxgen_edge{0};


	// taskflow object and executor
	tf::Taskflow _taskflow;
	tf::Executor _executor;

	// list to record to-be-updated vertices 
	std::vector<size_t> _to_update;

	// global suffix tree with a super target
	std::optional<Sfxt> _global_sfxt{std::nullopt};


	// prefix nodes from the last report iteration
	std::vector<std::unique_ptr<PfxtNode>> _pfxt_nodes;
	std::vector<std::unique_ptr<PfxtNode>> _pfxt_paths;
	std::vector<PfxtNode*> _pfxt_srcs;

	// leader prefix nodes
	// NOTE: we only store a raw pointer, an OBSERVER
	// because multiple prefix tree nodes may refer to this
	// observer and construct prefix tree nodes using the
	// observer's information
	std::vector<std::array<PfxtNode*, NUM_WEIGHTS>> _leaders;

	// leader nodes (not lookup table)
	std::vector<PfxtNode*> _leader_nodes;

	
	// dfs marked vector
	// to record the nodes that: 
	// 1. became sfxt nodes 
	//				or 
	// 2. updated but remained pfxt nodes
	std::vector<PfxtNode*> _dfs_marked;

	// dfs vector (full dfs traversal)
	// for subtree lookup
	std::vector<PfxtNode*> _dfs_full;

	// paths (unsorted, should sort when we finish exploring paths)
	std::vector<std::unique_ptr<Path>> _all_paths;

	// pfxt nodes affected
	std::vector<PfxtNode*> _affected_pfxtnodes;

	// re-spur list
	std::vector<Respur> _respurs;

	// num of affected nodes
	size_t _num_affected_nodes{0};
	
	// random device (for picking random vertices to updated)
	std::random_device _rdev;
	std::mt19937 _rng{_rdev()};

	// containers for all path costs
	std::vector<float> _all_path_costs;
	
	// search space expansion count
	size_t _sses{0};

	// subtree propagations
	size_t _props{0};

	// elapsed time: whole spur function
	size_t _elapsed_time_spur{0};
	size_t _elapsed_time_spur2{0};

	// elapsed time: identify leaders
	size_t _elapsed_time_idl{0};
	
	// elapsed time: spur loop
	size_t _elapsed_time_sploop{0};

	// elapsed time: transfer leftover nodes
	size_t _elapsed_time_tr{0};

	size_t _elapsed_final_path_sort{0};
	size_t _elapsed_init{0};

	size_t _elapsed_prop{0};
	size_t _elapsed_sse{0};
	size_t _elapsed_pop{0};
	size_t _total_nodes{0};
	size_t _skip_heaps{0};
	size_t _max_precomp_nodes{0};
	size_t _max_nodes{0};
	size_t _sort_cnt{0};
	size_t _leader_cnt;
};

/**
@brief Point Struct
*/
struct Point {
	friend class Ink;
	Point(const Vert& v, float d);
	Point(Point&&) = default;

	const Vert& vert;
	float dist;
};


/**
@brief Path Struct
*/
struct Path : std::list<Point> {
	Path(float w, const Point* endpt);
	Path(Path&&) = default;
	Path& operator = (Path&&) = default;

	Path(const Path&) = delete;
	Path& operator = (const Path&) = delete;

	void dump(std::ostream& os) const;

	float weight{std::numeric_limits<float>::quiet_NaN()};

	const Point* endpoint{nullptr};
};

/**
@brief Path Heap Class
A max heap to maintain top-k critical paths
*/
class PathHeap {
	friend class Ink;
	
	struct PathComp {
		bool operator () (
			const std::unique_ptr<Path>& a, 
			const std::unique_ptr<Path>& b) const {
			return a->weight < b->weight;
		}
	};

public:
	PathHeap() = default;
	PathHeap(PathHeap&&) = default;
	PathHeap& operator = (PathHeap&&) = default;

	PathHeap(const PathHeap&) = delete;
	PathHeap& operator = (const PathHeap&) = delete;

	inline size_t size() const;
	inline bool empty() const;

	void push(std::unique_ptr<Path> path);
	void pop();
	void merge_and_fit(PathHeap&& rhs, size_t K);
	void fit(size_t K);
	void heapify();

	Path* top() const;

	/**
	@brief sort and extract the paths in ascending order
	*/
	std::vector<Path> extract();
	void dump(std::ostream& os) const;

private:
	PathComp _comp;
	std::vector<std::unique_ptr<Path>> _paths;
};


} // end of namespace ink


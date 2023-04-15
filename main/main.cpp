#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "usage: ./Ink [graph_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	//ink.read_graph(argv[1]);
	
	// TODO: N vertices start from 0 to N-1
	// super source N, super target N+1
	// a total of N+2 vertices
	// NOTE: super src/dst are used implicitly
	ink.insert_edge("v1", "v2",
		0, 0, 0, 0,
		0, 0, 0, 0);
	
	ink.insert_edge("v2", "v4",
		0, 0, 0, 0,
		0, 0, 0, 0);

	ink.remove_edge("v1", "v2");

	ink.insert_edge("v6", "v7",
		0, 0, 0, 0,
		0, 0, 0, 0);

	ink.insert_edge("v8", "v10",
		0, 0, 0, 0,
		0, 0, 0, 0);

	ink.remove_edge("v2", "v4");

	ink.remove_vertex("v6");

	ink.remove_vertex("v8");
	ink.dump(std::cout);

	return 0;
}

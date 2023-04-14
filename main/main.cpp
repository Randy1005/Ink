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
	
	ink.insert_vertex("v1");

	ink.insert_edge("v1", "v2",
		0, std::nullopt, std::nullopt, 0,
		std::nullopt, 0, 0, 0);



	ink.insert_edge("v1", "v2",
		0, 1.25, 3.78, 0,
		3.08, 0, 0, 0);


	ink.dump(std::cout);
	return 0;
}

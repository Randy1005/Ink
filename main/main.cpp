#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "usage: ./Ink [graph_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_graph(argv[1]);

	ink.remove_edge("u1:A", "u1:Y");

	ink.dump(std::cout);

	// TODO: N vertices start from 0 to N-1
	// super source N, super target N+1
	// a total of N+2 vertices
	// NOTE: super src/dst are used implicitly
	return 0;
}

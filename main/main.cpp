#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "usage: ./Ink [graph_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_graph(argv[1]);


	ink.remove_vertex("kkk");
	ink.remove_vertex("f1:");

	ink.dump(std::cout);


	return 0;
}

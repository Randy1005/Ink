#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "usage: ./Ink [graph_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_graph(argv[1]);



	ink.remove_vertex("f1:");
	ink.remove_vertex("u2:A");
	ink.remove_vertex("u4:Y");
	ink.insert_edge("u4:A", "f1:D", 0, 0, 0, 0, 0, 0, 0 ,0);

	ink.dump(std::cout);


	return 0;
}

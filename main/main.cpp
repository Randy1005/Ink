#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "usage: ./Ink [graph_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_graph(argv[1]);

	ink.create_super_dst("SUPER_DST");
	ink.build_sfxt();

	ink.dump(std::cout);

	return 0;
}

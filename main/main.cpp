#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "usage: ./Ink [graph_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_graph(argv[1]);

	auto paths = ink.report(50);
	for (const auto& p : paths) {
		p.dump(std::cout);
	}	
	
	//ink.dump(std::cout);

	return 0;
}

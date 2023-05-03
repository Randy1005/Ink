#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 3) {
		std::cerr << "usage: ./Ink [graph_ops_file] [output_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.read_ops_and_report(argv[1], argv[2]);

	
	return 0;
}

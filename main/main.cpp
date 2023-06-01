#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 4) {
		std::cerr << "usage: ./Ink [graph_ops_file] [output_file] [report_mode]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	auto mode = std::stoi(argv[3]);
	ink.read_ops_and_report(argv[1], argv[2], mode);

	
	return 0;
}

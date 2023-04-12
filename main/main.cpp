#include <ink/ink.hpp>

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "usage: ./Ink [graph_file]\n";
		std::exit(EXIT_FAILURE);
	}

	ink::Ink ink;
	ink.insert_vertex("sample");
	ink.insert_vertex("sample2");

	ink.insert_edge(
		"sample", 
		"sample2",
		1.0, 1.0, 1.0, 2.0,
		std::nullopt, 3.0, 1.0, 4.9);
	
	ink.insert_edge(
		"s3", 
		"s4",
		1.5, 1.55, 1.89, 2.0,
		std::nullopt, std::nullopt, 1.0, 4.9);


	ink.dump(std::cout);


	return 0;
}

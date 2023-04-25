#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

TEST_CASE("Simple Chain Graph" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("S", "A",
		2, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("A", "B",
		3, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	
	ink.insert_edge("B", "C",
		4, 4, 4, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	
	ink.insert_edge("C", "T",
		5, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	
	
	auto paths = ink.report(10);

	for (const auto& p : paths) {
		p.dump(std::cout);
	}

}

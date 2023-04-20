#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <ink/ink.hpp>

TEST_CASE("Sample Graph From UITimer Paper" * doctest::timeout(300)) {
	ink::Ink ink;

	ink.insert_edge("v0", "v6",
		2, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v0", "v4",
		4, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v1", "v4",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v2", "v4",
		6, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	ink.insert_edge("v2", "v5",
		5, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v3", "v6",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	

	ink.insert_edge("v4", "v6",
		3, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v4", "v7",
		2, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v5", "v7",
		1, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v6", "v8",
		6, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	ink.insert_edge("v7", "v8",
		3, std::nullopt, std::nullopt, std::nullopt,
		std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	
	auto paths = ink.report(10);

	for (const auto& p : paths) {
		p.dump(std::cout);
	}

}

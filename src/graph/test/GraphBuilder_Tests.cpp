#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <random>

using namespace std;
using namespace graph;
using namespace graphUtil;

TEST(GraphBuilder, Correctness) {
	using EmptyStaticGraph = StaticGraph<Properties<>>;

	int n = 100;
	int m = 1000;
	vector<pair<int, int>> possibleEdges;
	for (auto& v : Range(0, n)) {
		for (auto& to : Range(0, n)) {
			if (v == to) continue;
			possibleEdges.push_back(make_pair(v, to));
		}
	}

	random_device random_device;
	mt19937 generator(random_device());
	shuffle(possibleEdges.begin(), possibleEdges.end(), generator);
	possibleEdges.erase(possibleEdges.begin() + m, possibleEdges.end());

	auto g = EmptyStaticGraph(possibleEdges.begin(), possibleEdges.end(), n, m);

	EXPECT_EQ(num_vertices(g), n);
	EXPECT_EQ(num_edges(g), m);

	auto graphVertices = SortedValuesRange(vertices(g));
	EXPECT_TRUE(graphVertices.contains(0));
	EXPECT_FALSE(graphVertices.contains(n));

	for (auto& v : graphVertices) {
		auto inAdjacencies = Range(in_adjacent_vertices(v, g));
		EXPECT_TRUE(std::is_sorted(inAdjacencies.begin(), inAdjacencies.end()));
		auto outAdjacencies = Range(adjacent_vertices(v, g));
		EXPECT_TRUE(std::is_sorted(outAdjacencies.begin(), outAdjacencies.end()));

		for (const auto& e : Range(out_edges(v, g))) {
			auto to = target(e, g);
			auto toInEdges = ValuesRange(in_edges(to, g));
			EXPECT_TRUE(toInEdges.contains(e));
		}

		for (const auto& e : Range(in_edges(v, g))) {
			auto to = source(e, g);
			auto toOutEdges = ValuesRange(out_edges(to, g));
			EXPECT_TRUE(toOutEdges.contains(e));
		}
	}
}

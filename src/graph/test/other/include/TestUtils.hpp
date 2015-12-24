#pragma once
#include <vector>
#include <random>
#include <graph/detail/util/Collection.hpp>
#include <gtest/gtest.h>

inline std::vector<std::pair<size_t, size_t>> GenerateRandomEdgeList(int verticesCount, int edgesCount) {
	using namespace std;
	using namespace graphUtil;
	vector<pair<size_t, size_t>> possibleEdges;
	for (size_t v : Range(0, verticesCount)) {
		for (size_t to : Range(0, verticesCount)) {
			if (v == to) continue;
			possibleEdges.push_back(make_pair(v, to));
		}
	}

	random_device random_device;
	mt19937 generator(random_device());
	shuffle(possibleEdges.begin(), possibleEdges.end(), generator);
	possibleEdges.erase(possibleEdges.begin() + edgesCount, possibleEdges.end());
	return possibleEdges;
}

template<typename Graph>
inline void VerifyGraphStructure(Graph &g, int n, int m) {
	using namespace graphUtil;

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

		EXPECT_EQ(Range(out_edges(v, g)).size(), out_degree(v, g));
		for (const auto& e : Range(out_edges(v, g))) {
			auto to = target(e, g);
			auto toInEdges = ValuesRange(in_edges(to, g));
			EXPECT_TRUE(toInEdges.contains(e));
		}

		EXPECT_EQ(Range(in_edges(v, g)).size(), in_degree(v, g));
		for (const auto& e : Range(in_edges(v, g))) {
			auto to = source(e, g);
			auto toOutEdges = ValuesRange(out_edges(to, g));
			EXPECT_TRUE(toOutEdges.contains(e));
		}
	}
}
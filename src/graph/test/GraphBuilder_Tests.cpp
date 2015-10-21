#pragma once

#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <random>

using namespace std;
using namespace graph;

TEST(GraphBuilder, Correctness) {
	int n = 1000;
	int m = 100000;
	vector<pair<int, int>> possibleEdges;
	for (int v = 0; v < n; ++v) {
		for (int to = 0; to < n; ++to) {
			if (v == to) continue;
			possibleEdges.push_back(make_pair(v, to));
		}
	}

	random_device random_device;
	mt19937 generator(random_device());
	shuffle(possibleEdges.begin(), possibleEdges.end(), generator);
	possibleEdges.erase(possibleEdges.begin() + m, possibleEdges.end());

	auto g = StaticGraph(possibleEdges.begin(), possibleEdges.end(), n, m);

	EXPECT_EQ(g.Vertices().size(), n);
	EXPECT_EQ(g.EdgesCount(), m);

	for (auto& v : g.Vertices()) {
		auto inAdjacencies = g.InAdjacencies(v);
		EXPECT_TRUE(std::is_sorted(inAdjacencies.begin(), inAdjacencies.end()));
		auto outAdjacencies = g.OutAdjacencies(v);
		EXPECT_TRUE(std::is_sorted(outAdjacencies.begin(), outAdjacencies.end()));

		auto outEdges = g.OutEdges(v);
		for (auto& e : outEdges) {
			auto to = target(e, g);
			auto toInEdges = g.InEdges(to);
			bool contains = find(toInEdges.begin(), toInEdges.end(), e) != toInEdges.end();
			EXPECT_TRUE(contains);
		}

		auto inEdges = g.InEdges(v);
		for (auto& e : inEdges) {
			auto to = target(e, g);
			auto toInEdges = g.InEdges(to);
			bool contains = find(toInEdges.begin(), toInEdges.end(), e) != toInEdges.end();
			EXPECT_TRUE(contains);
		}
	}
}

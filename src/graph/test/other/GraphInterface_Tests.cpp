#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <graph/detail/ComplementGraph.hpp>
#include <graph/detail/IncidenceGraph.hpp>
#include <graph/detail/util/Collection.hpp>
#include <graph/dynamic/DynamicGraph.hpp>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;
using namespace graph;
using namespace graphUtil;

struct SomeVertexPropertyType {};

struct SomeEdgePropertyType {};

using VertexProperties = Properties<Property<SomeVertexPropertyType, uint32_t>>;
using EdgeProperties = Properties<Property<SomeEdgePropertyType, uint32_t>>;
using Graph = StaticGraph<VertexProperties, EdgeProperties>;
using DGraph = DynamicGraph<VertexProperties, EdgeProperties>;

inline vector<pair<size_t, size_t>> GenerateRandomEdgeList(int verticesCount, int edgesCount) {
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

template <typename Graph>
inline void VerifyGraphStructure(Graph& g, int n, int m) {
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

template <typename Graph>
inline void VerifyIncidenceGraphStructure(Graph& g, int n, int m) {
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
		EXPECT_EQ(Range(in_edges(v, g)).size(), in_degree(v, g));
	}
}


template <typename Graph>
inline void VerifyPropertyMaps(Graph& g, int n, int m) {
	auto vertexIndex = graph::get(vertex_index_t(), g);
	auto someVertexProperty = graph::get(SomeVertexPropertyType(), g);
	auto someEdgeProperty = graph::get(SomeEdgePropertyType(), g);

	typename property_map<Graph, vertex_index_t>::type::value_type vertexId = 0;
	for (const auto& v : Range(vertices(g))) {
		put(someVertexProperty, v, get(vertexIndex, v));
		EXPECT_EQ(vertexId, get(someVertexProperty, v));
		++vertexId;

		typename property_map<Graph, SomeEdgePropertyType>::type::value_type edgeId;
		for (const auto& e : Range(out_edges(v, g))) {
			put(someEdgeProperty, e, edgeId);
			EXPECT_EQ(edgeId, get(someEdgeProperty, e));
			++edgeId;
		}
	}
}

TEST(GraphInterface, GraphBuilderCorrectness) {
	int n = 100;
	int m = 1000;

	auto edgeList = GenerateRandomEdgeList(n, m);
	auto g = Graph(edgeList.begin(), edgeList.end(), n, m);

	VerifyGraphStructure(g, n, m);
	VerifyPropertyMaps(g, n, m);
}

TEST(GraphInterface, ComplementGraphValidation) {

	using Graph = StaticGraph<VertexProperties, EdgeProperties>;
	using CGraph = ComplementGraph<Graph>;

	int n = 100;
	int m = 1000;

	auto edgeList = GenerateRandomEdgeList(n, m);
	auto graph = Graph(edgeList.begin(), edgeList.end(), n, m);
	auto cGraph = CGraph(graph);

	VerifyGraphStructure(cGraph, n, m);
	VerifyPropertyMaps(cGraph, n, m);
}

TEST(GraphInterface, DynamicGraphCorrectness) {
	size_t n = 100;
	size_t m = 1000;

	auto edgeList = GenerateRandomEdgeList(n, m);
	vector<vector<size_t>> edges;
	edges.resize(n);
	for (auto e : edgeList) {
		edges[e.first].push_back(e.second);
	}
	for (auto& vEdges : edges) {
		stable_sort(vEdges.begin(), vEdges.end());
	}

	auto g = DGraph(edgeList.begin(), edgeList.end(), n, m);

	EXPECT_EQ(num_vertices(g), n);
	EXPECT_EQ(num_edges(g), m);

	auto graphVertices = SortedValuesRange(vertices(g));
	EXPECT_TRUE(graphVertices.contains(0));
	EXPECT_FALSE(graphVertices.contains(n));
	for (auto& v : graphVertices) {
		auto outAdjacencies = AsArray(Range(adjacent_vertices(v, g)));
		auto outEdges = Range(out_edges(v, g));
		EXPECT_EQ(outAdjacencies.size(), outEdges.size());
		EXPECT_EQ(outEdges.size(), out_degree(v, g));
		int idx = 0;
		for(const auto& e : outEdges) {
			EXPECT_EQ(v, source(e, g));
			EXPECT_EQ(outAdjacencies[idx++], target(e, g));
		}

		stable_sort(outAdjacencies.begin(), outAdjacencies.end());
		EXPECT_TRUE(std::equal(edges[v].begin(), edges[v].end(), outAdjacencies.begin()));
	}
	VerifyPropertyMaps(g, n, m);
}

TEST(GraphInterface, IncidenceGraphValidation) {
	using VertexProperties = Properties<Property<SomeVertexPropertyType, int>>;
	using EdgeProperties = Properties<Property<SomeEdgePropertyType, int>>;
	using Graph = StaticGraph<VertexProperties, EdgeProperties>;
	using IGraph = IncidenceGraph<Graph>;

	int n = 100;
	int m = 1000;

	auto edgeList = GenerateRandomEdgeList(n, m);
	auto graph = Graph(edgeList.begin(), edgeList.end(), n, m);
	auto iGraph = IGraph(graph);

	VerifyIncidenceGraphStructure(iGraph, n, m);
	VerifyPropertyMaps(iGraph, n, m);
}


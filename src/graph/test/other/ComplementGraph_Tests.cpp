#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <graph/detail/ComplementGraph.hpp>
#include <random>
#include <TestUtils.hpp>

using namespace std;
using namespace graph;
using namespace graphUtil;

struct SomeVertexPropertyType{};
struct SomeEdgePropertyType{};

TEST(ComplementGraph, Concepts) {
	using VertexProperties = Properties<Property<SomeVertexPropertyType, int>>;
	using EdgeProperties = Properties<Property<SomeEdgePropertyType, int>>;
	using Graph = StaticGraph<VertexProperties, EdgeProperties>;
	using CGraph = ComplementGraph<Graph>;

	int n = 100;
	int m = 1000;

	auto edgeList = GenerateRandomEdgeList(n, m);
	auto graph = Graph(edgeList.begin(), edgeList.end(), n, m);
	auto cGraph = CGraph(graph);

	VerifyGraphStructure(cGraph, n, m);

	auto vertexIndex = graph::get(vertex_index_t(), cGraph);	
	auto someVertexProperty = graph::get(SomeVertexPropertyType(), cGraph);
	auto someEdgeProperty = graph::get(SomeEdgePropertyType(), cGraph);
	
	property_map<CGraph, vertex_index_t>::type::value_type vertexId = 0;
	for(const auto& v : Range(vertices(graph))) {
		put(someVertexProperty, v, get(vertexIndex, v));
		EXPECT_EQ(vertexId, get(someVertexProperty, v));
		++vertexId;

		property_map<CGraph, SomeEdgePropertyType>::type::value_type edgeId;
		for(const auto& e : Range(out_edges(v, cGraph))) {
			put(someEdgeProperty, e, edgeId);
			EXPECT_EQ(edgeId, get(someEdgeProperty, e));
			++edgeId;
		}
	}
}

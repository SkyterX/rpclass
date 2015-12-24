#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <random>
#include <TestUtils.hpp>

using namespace std;
using namespace graph;
using namespace graphUtil;

TEST(GraphBuilder, Correctness) {
	using EmptyStaticGraph = StaticGraph<>;

	int n = 100;
	int m = 1000;
	
	auto edgeList = GenerateRandomEdgeList(n, m);
	auto g = EmptyStaticGraph(edgeList.begin(), edgeList.end(), n, m);

	VerifyGraphStructure(g, n, m);
}

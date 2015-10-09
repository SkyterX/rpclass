#include <type_traits>
#include <utility>
#include<cassert>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <graph/graph_traits.hpp>
#include "generator.hpp"


using namespace std;
using namespace graph;

TEST(StaticGraph, Constructor) {
    StaticGraph staticGraph;
}

TEST(StaticGraph, IncedenceGraphConcept) {
    using Graph = boost::adjacency_list<>;
    BOOST_CONCEPT_ASSERT((boost::concepts::GraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraphConcept<Graph>));
    SUCCEED();

};


TEST(StaticGraph, ListGraph) {
    using Graph = boost::adjacency_list<>;
    using SizeT = uint32_t;
    using VecPair = vector<pair<SizeT, SizeT>>;
    const SizeT n = 1 << 16;
    VecPair input;
    input.reserve(n);
    back_insert_iterator<VecPair> backInserter(input);
    generate_list_graph(backInserter, n);
    Graph graph(input.begin(), input.end(), n, n);
    //count # of cycles using vector    
    vector<SizeT> cycles;
    for (auto p = input.begin(); p != input.end(); ++p) {
        if (p->first != n) {
            p->first = n;
            SizeT length = 1;
            auto cycleIt = p;            
            while ((cycleIt = input.begin() + cycleIt->second) != p) {
                cycleIt->first = n;
                ++length;
            }
            cycles.push_back(length);
        }
    }
    sort(cycles.begin(), cycles.end());
    
    //count # of cycles using graph data structure    
    // assumes that the vertex_descriptor is an integral in [0,n)
    char* visited = new char[n];
    memset(visited, 0, sizeof(char)*n);
    graph_traits<Graph>::vertices_size_type numVertices = num_vertices(graph);
    EXPECT_EQ(n, numVertices);
    graph_traits<Graph>::edges_size_type numEdges = 0;
    vector<graph_traits<Graph>::vertices_size_type> graphCycles;
    auto vRange = vertices(graph);
    for (graph_traits<Graph>::vertex_iterator vIt = vRange.first; vIt != vRange.second; ++vIt) {
        if (!visited[*vIt]) {
            visited[*vIt] = true;
            graph_traits<Graph>::degree_size_type degree = out_degree(*vIt, graph);
            EXPECT_EQ(1, degree);
            numEdges += degree;
            graph_traits<Graph>::vertices_size_type length = 1;
            auto outRange = adjacent_vertices(*vIt, graph);
            graph_traits<Graph>::vertex_descriptor curVertex;
            while ( (curVertex =*outRange.first) != *vIt) {
                ++length;
                visited[curVertex] = true;
                outRange = adjacent_vertices(curVertex, graph);
                degree = out_degree(curVertex, graph);
                EXPECT_EQ(1, degree);
                numEdges += degree;
            }
            graphCycles.push_back(length);
        }
    }
    delete[] visited;
    sort(graphCycles.begin(), graphCycles.end());
    EXPECT_EQ(cycles.size(), graphCycles.size());
    for (SizeT i = 0; i < cycles.size(); ++i)
        EXPECT_EQ(cycles[i], graphCycles[i]);
    assert(numEdges == n);
    EXPECT_EQ(n, num_edges(graph));    
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
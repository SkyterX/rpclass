#include <type_traits>
#include <utility>
#include <cassert>
#include <queue>
#include <gtest/gtest.h>
#include <boost/graph/graph_concepts.hpp>
#include <graph/static_graph.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/breadth_first_search.hpp>
#include <graph/dijkstra.hpp>
#include <graph/bidirectional_dijkstra.hpp>
#include <graph/io.hpp>
#include <generator.hpp>

using namespace std;
using namespace graph;

//BFS related properties
struct distance_t {};
struct color_t {};
struct edge_type_t {};

using BFSBundledVertexProperties = Properties<Property<distance_t, uint32_t>, Property<color_t, char>>;
using BFSBundledEdgeProperties = Properties<Property<edge_type_t, char>>;

TEST(GraphConcepts, GraphConcept) {
    using Graph = StaticGraph<>;
    BOOST_CONCEPT_ASSERT((boost::concepts::GraphConcept<Graph>));
};

TEST(GraphConcepts, VertexListGraphConcept) {
    using Graph = StaticGraph<>;
    BOOST_CONCEPT_ASSERT((boost::concepts::VertexListGraphConcept<Graph>));
};

TEST(GraphConcepts, AdjacencyGraphConcept) {
    using Graph = StaticGraph<>;
	BOOST_CONCEPT_ASSERT((boost::concepts::AdjacencyGraphConcept<Graph>));
};

TEST(GraphConcepts, IncidenceGraphConcept) {
    using Graph = StaticGraph<>;
    BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraphConcept<Graph>));
};

TEST(GraphConcepts, BidirectionalGraphConcept) {
    using Graph = StaticGraph<BFSBundledVertexProperties, BFSBundledEdgeProperties>;
    BOOST_CONCEPT_ASSERT((boost::concepts::BidirectionalGraphConcept<Graph>));
};

TEST(PropertyGraph, InternalProperties) {
    using Graph = StaticGraph<BFSBundledVertexProperties, BFSBundledEdgeProperties>;

    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<Graph, graph::vertex_bundle_t>::type,
        graph::property_map<Graph, graph::vertex_bundle_t>::type::key_type>));
    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<Graph, graph::edge_bundle_t>::type,
        graph::property_map<Graph, graph::edge_bundle_t>::type::key_type>));
};

TEST(PropertyGraph, InternalColorEdgeTypeProperties) {
    using Graph = StaticGraph<BFSBundledVertexProperties, BFSBundledEdgeProperties>;

    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<Graph, color_t>::type,
        graph::property_map<Graph, color_t>::type::key_type>));
    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<Graph, edge_type_t>::type,
        graph::property_map<Graph, edge_type_t>::type::key_type>));
};

//TEST(PropertyGraph, VertexIndexProperty) {
//    using Graph = BFSGraph;
//    using Vertex = typename graph_traits<Graph>::vertex_descriptor;
//    using Map = typename property_map<Graph, graph::vertex_index_t>::type;
//    using Index = Graph::vertices_size_type;
//    Map m = get(vertex_index_t{}, g);
//    Index x = get(vertex_index_t{}, g, Vertex())
//};

TEST(GraphStructure, ListGraph) {
    using Graph = StaticGraph<>;

    using SizeT = size_t;
    using VecPair = vector<pair<SizeT, SizeT>>;
    const SizeT n = 1 << 2;
    VecPair input;
    input.reserve(n);
    back_insert_iterator<VecPair> backInserter(input);
    generate_list_graph(backInserter, n);
    Graph graph(input.begin(), input.end(), n);
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
            auto outRange = out_edges(*vIt, graph);
            graph_traits<Graph>::vertex_descriptor curVertex;
            while ( (curVertex = target(*outRange.first, graph)) != *vIt) {
                ++length;
                visited[curVertex] = true;
                outRange = out_edges(curVertex, graph);
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
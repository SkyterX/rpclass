#include <type_traits>
#include <utility>
#include <cassert>
#include <queue>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/property_map/property_map.hpp>
#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include "generator.hpp"


using namespace std;
using namespace graph;

//Graph without any internal properties
using NoInternalPropertiesGraph = boost::adjacency_list<>;
namespace graph {
    template<>
    struct graph_traits<NoInternalPropertiesGraph> :
        boost::graph_traits<NoInternalPropertiesGraph> {};
}


//BFS graph and related structures 
struct distance_t {};
struct color_t {};
struct edge_type_t {};
using namespace graph;
using BFSBundledVertexProperties = Properties<Property<distance_t, uint32_t>, Property<color_t, char>>;
using BFSBundledEdgeProperties = Properties<Property<edge_type_t, char>>;
using BFSGraph = boost::adjacency_list<
    boost::vecS, boost::vecS, boost::bidirectionalS,
    BFSBundledVertexProperties, BFSBundledEdgeProperties>;

namespace graph {
    template<>
    struct graph_traits<BFSGraph> : boost::graph_traits<BFSGraph> {};

    template<>
    struct property_map<BFSGraph, vertex_bundle_t> {
        using type = boost::property_map<BFSGraph, boost::vertex_bundle_t>::type;
    };

    template<>
    struct property_map<BFSGraph, edge_bundle_t> {
        using type = boost::property_map<BFSGraph, boost::edge_bundle_t>::type;
    };

    template<>
    inline graph::property_map<BFSGraph, vertex_bundle_t>::type
        get<BFSGraph>(const vertex_bundle_t&, BFSGraph& graph) {
        return boost::get(boost::vertex_bundle, graph);
    };

    template<>
    inline property_map<BFSGraph, edge_bundle_t>::type
        get<BFSGraph>(const edge_bundle_t&, BFSGraph& graph) {
        return boost::get(boost::edge_bundle, graph);
    }

    template<>
    inline property_map<BFSGraph, vertex_bundle_t>::type::reference
        get<property_map<BFSGraph, vertex_bundle_t>::type>(const property_map<BFSGraph, vertex_bundle_t>::type& pMap,
            const property_map<BFSGraph, vertex_bundle_t>::type::key_type& key) {
        return boost::get(pMap, key);
    };

    template<>
    inline property_map<BFSGraph, edge_bundle_t>::type::reference
        get<property_map<BFSGraph, edge_bundle_t>::type>(const property_map<BFSGraph, edge_bundle_t>::type& pMap,
            const property_map<BFSGraph, edge_bundle_t>::type::key_type& key) {
        return boost::get(pMap, key);
    };

    template<>
    inline void put<property_map<BFSGraph, vertex_bundle_t>::type> (property_map<BFSGraph, vertex_bundle_t>::type& pMap,
        const property_map<BFSGraph, vertex_bundle_t>::type::key_type& key, 
        const property_map<BFSGraph, vertex_bundle_t>::type::value_type& value) {
        boost::put(pMap, key, value);
    };

    template<>
    inline void put<property_map<BFSGraph, edge_bundle_t>::type>(property_map<BFSGraph, edge_bundle_t>::type& pMap,
        const property_map<BFSGraph, edge_bundle_t>::type::key_type& key,
        const property_map<BFSGraph, edge_bundle_t>::type::value_type& value) {
        boost::put(pMap, key, value);
    };

}

TEST(GraphConcepts, GraphConcept) {
    using Graph = BFSGraph;
    BOOST_CONCEPT_ASSERT((boost::concepts::GraphConcept<Graph>));
};

TEST(GraphConcepts, VertexListGraphConcept) {
    using Graph = BFSGraph;
    BOOST_CONCEPT_ASSERT((boost::concepts::VertexListGraphConcept<Graph>));
};

TEST(GraphConcepts, IncidenceGraphConcept) {
    using Graph = BFSGraph;
    BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraphConcept<Graph>));
};

TEST(GraphConcepts, BidirectionalGraphConcept) {
    using Graph = BFSGraph;
    BOOST_CONCEPT_ASSERT((boost::concepts::BidirectionalGraphConcept<Graph>));
};


TEST(PropertyGraph, InternalProperties) {
    using Graph = BFSGraph;
    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<BFSGraph, graph::vertex_bundle_t>::type, 
        graph::property_map<BFSGraph, graph::vertex_bundle_t>::type::key_type>));
    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<BFSGraph, graph::edge_bundle_t>::type, 
        graph::property_map<BFSGraph, graph::edge_bundle_t>::type::key_type>));
};

TEST(PropertyGraph, InternalColorEdgeTypeProperties) {
    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<BFSGraph, color_t>::type,
        graph::property_map<BFSGraph, color_t>::type::key_type>));
    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<
        graph::property_map<BFSGraph, edge_type_t>::type,
        graph::property_map<BFSGraph, edge_type_t>::type::key_type>));
};


TEST(GraphStructure, ListGraph) {
    using Graph = NoInternalPropertiesGraph;

    using SizeT = uint32_t;
    using VecPair = vector<pair<SizeT, SizeT>>;
    const SizeT n = 1 << 10;
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


TEST(GraphAlgorithms, BFS) {
    using Graph = BFSGraph;
    using SizeT = uint32_t;
    using VecPair = vector<pair<SizeT, SizeT>>;
    const SizeT n = 1 << 16;
    VecPair input;
    input.reserve(n);
    back_insert_iterator<VecPair> backInserter(input);
    generate_list_graph(backInserter, n);
    Graph graph(input.begin(), input.end(), n);
    auto colorPM = graph::get(color_t(), graph);
    auto distancePM = graph::get(distance_t(), graph);
    auto edgeTypePM = graph::get(edge_type_t(), graph);
    auto vRange = vertices(graph);
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt)
        graph::put(colorPM, *vIt, 0);
    std::queue<graph_traits<Graph>::vertex_descriptor> vQueue;
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
        if (graph::get(colorPM, *vIt) == 0) {
            auto start = *vIt;
            vQueue.push(start);
            while (!vQueue.empty()) {
                auto src = vQueue.front();
                vQueue.pop();
                auto outRange = out_edges(src, graph);
                graph::put(colorPM, src, 2);
                for (auto outIt = outRange.first; outIt != outRange.second; ++outIt) {
                    auto e = *outIt;
                    auto tgt = target(e,graph);
                    if (graph::get(colorPM, tgt) == 0) {
                        graph::put(colorPM, tgt, 1);
                        graph::put(distancePM, tgt, graph::get(distancePM, tgt) + 1);
                        graph::put(edgeTypePM, e, 0);
                        vQueue.push(tgt);
                    }
                    else
                        if (graph::get(colorPM, tgt) == 1) graph::put(edgeTypePM, e, 1);
                        else  graph::put(edgeTypePM, e, 2);
                }
            }
        }
    };
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
        EXPECT_EQ(2,graph::get(colorPM, *vIt));
        EXPECT_LT(graph::get(distancePM, *vIt), n);
    };
};


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
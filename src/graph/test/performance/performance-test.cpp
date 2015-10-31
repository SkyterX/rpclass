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

struct distance_t {};
struct distanceB_t {};
struct color_t {};
struct edge_type_t {};
struct predecessor_t {};
struct predecessorB_t {};
struct weight_t {};

char* globalInputGraphFile = nullptr;

template <typename Graph, typename PropertyMap>
struct MyBFSVisitor
    : graph::DefaultBFSVisitor<Graph, typename property_map<Graph, color_t>::type> {
    using Base = graph::DefaultBFSVisitor<Graph, typename property_map<Graph, color_t>::type>;
    MyBFSVisitor(Graph& graph,
        const typename graph_traits<Graph>::vertex_descriptor& s)
        : Base(graph::get(color_t(), graph)),
        distance(graph::get(distance_t(), graph)),
        edgeType(graph::get(edge_type_t(), graph)) {
        put(distance, s, 0);
    };
    void initialize_vertex(const typename graph_traits<Graph>::vertex_descriptor&, Graph&) {};
    void discover_vertex(const typename graph_traits<Graph>::vertex_descriptor& u, Graph&) {};
    void tree_edge(const typename graph_traits<Graph>::edge_descriptor& e, Graph& g) {
        graph::put(distance, target(e,g), graph::get(distance, source(e,g)) + 1);
        graph::put(edgeType, e, 0);
    };
    void gray_target(const typename graph_traits<Graph>::edge_descriptor& e, Graph&) {
        graph::put(edgeType, e, 1);
    };
    void black_target(const typename graph_traits<Graph>::edge_descriptor& e, Graph&) {
        graph::put(edgeType, e, 2);
    };
    typename graph::property_map<Graph, distance_t>::type distance;
    typename graph::property_map<Graph, edge_type_t>::type edgeType;
};

TEST(GraphAlgorithms, BFS) {
    using  Graph = GenerateBFSGraph<color_t,
        Properties<Property<distance_t, uint32_t>>,
        Properties<Property<edge_type_t, char>>>::type;
    using DdsgVecType = std::vector<std::pair<std::pair<Graph::vertices_size_type,
        Graph::vertices_size_type>, Property<weight_t, double >> >;
    DdsgVecType ddsgVec;
    back_insert_iterator<DdsgVecType> ddsgVecBackInserter(ddsgVec);
    size_t numOfNodes;
    size_t numOfEdges;
    read_ddsg<Property<weight_t, double>>(ddsgVecBackInserter, numOfNodes, numOfEdges, globalInputGraphFile);
    std::sort(ddsgVec.begin(), ddsgVec.end(),
        [&](DdsgVecType::value_type left, DdsgVecType::value_type right) {
        return left.first.first < right.first.first;
    });
    Graph graph(ddsgVec.begin(), ddsgVec.end(), numOfNodes, numOfEdges);

    auto colorPM = graph::get(color_t(), graph);
    auto distancePM = graph::get(distance_t(), graph);
    using BFSVisitor = MyBFSVisitor<Graph, property_map<Graph, color_t>::type>;
    auto vRange = vertices(graph);
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
        graph::put(colorPM, *vIt, 0);
        graph::put(distancePM, *vIt, numOfNodes);
    };
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt)
        breadth_first_search(graph, *vIt, colorPM, BFSVisitor(graph,*vIt));

    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
        EXPECT_EQ(2, graph::get(colorPM, *vIt));
        EXPECT_LT(graph::get(distancePM, *vIt), numOfNodes);
    };
};

TEST(GraphAlgorithms, Dijkstra) {
    using Graph = GenerateDijkstraGraph<predecessor_t, distance_t, weight_t,
        vertex_index_t, color_t, Properties<>, Properties< >> ::type;
    using DdsgVecType = std::vector<std::pair<std::pair<Graph::vertices_size_type,
        Graph::vertices_size_type>, Property<weight_t, double >> >;
    DdsgVecType ddsgVec;
    back_insert_iterator<DdsgVecType> ddsgVecBackInserter(ddsgVec);
    size_t numOfNodes;
    size_t numOfEdges;
    read_ddsg<Property<weight_t, double>>(ddsgVecBackInserter, numOfNodes, numOfEdges, "globalInputGraphFile");
    std::sort(ddsgVec.begin(), ddsgVec.end(),
        [&](DdsgVecType::value_type left, DdsgVecType::value_type right) {
        return left.first.first < right.first.first;
    });
    Graph graph(ddsgVec.begin(), ddsgVec.end(), numOfNodes, numOfEdges);
    //    Graph graph;
    auto predecessor = get(predecessor_t(), graph);
    auto distance = get(distance_t(), graph);
    auto weight = get(weight_t(), graph);
    auto vertex_index = get(vertex_bundle_t(), graph);
    auto color = get(color_t(), graph);
    dijkstra(graph, graph_traits<Graph>::vertex_descriptor(), predecessor, distance, weight, vertex_index, color);
};

TEST(GraphAlgorithms, BiDijkstra) {
    using Graph = GenerateBiDijkstraGraph<predecessor_t, predecessorB_t,
        distance_t, distanceB_t, weight_t, vertex_index_t, color_t,
        Properties<>, Properties< >> ::type;
    Graph graph;
    auto predecessorF = get(predecessor_t(), graph);
    auto predecessorB = get(predecessorB_t(), graph);
    auto distanceF = get(distance_t(), graph);
    auto distanceB = get(distanceB_t(), graph);
    auto weight = get(weight_t(), graph);
    auto vertex_index = get(vertex_bundle_t(), graph);
    auto color = get(color_t(), graph);
    bidirectional_dijkstra(graph, graph_traits<Graph>::vertex_descriptor(),
        graph_traits<Graph>::vertex_descriptor(), predecessorF, predecessorB,
        distanceF, distanceB, weight, vertex_index, color);
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc == 2)  globalInputGraphFile = argv[1];
    else {
        cerr << ".ddsg input graph file is required" << endl;
        return 1;
    }
    return RUN_ALL_TESTS();
}
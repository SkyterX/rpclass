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
#include <vector>
#include <string>

using namespace std;
using namespace graph;

struct distance_t {};
struct distanceB_t {};
struct color_t {};
struct colorB_t {};
struct edge_type_t {};
struct predecessor_t {};
struct predecessorB_t {};
struct weight_t {};

char* globalPathToFiles = nullptr;

class DdsgGraphAlgorithm : public ::testing::TestWithParam<const char*> {
protected:
    DdsgGraphAlgorithm() :m_ddsgVecBackInserter(m_ddsgVec) {};
    virtual void SetUp() {
        string fileName(globalPathToFiles);
        fileName+=GetParam();        
        if (read_ddsg<Property<weight_t, uint32_t>>(m_ddsgVecBackInserter, m_numOfNodes, m_numOfEdges, fileName.c_str()))
            FAIL();        
        std::sort(m_ddsgVec.begin(), m_ddsgVec.end(),
            [&](DdsgVecType::value_type left, DdsgVecType::value_type right) {
            return left.first.first < right.first.first;
        });
    };
    using DdsgVecType = std::vector<std::pair<std::pair<size_t,size_t>,Property<weight_t, uint32_t>>>;
    DdsgVecType m_ddsgVec;
    back_insert_iterator<DdsgVecType> m_ddsgVecBackInserter;
    size_t m_numOfNodes;
    size_t m_numOfEdges;

};

template <typename Graph, typename PropertyMap>
struct CorrectnessBFSVisitor
    : graph::DefaultBFSVisitor<Graph, typename property_map<Graph, color_t>::type> {
    using Base = graph::DefaultBFSVisitor<Graph, typename property_map<Graph, color_t>::type>;
    CorrectnessBFSVisitor(Graph& graph,
        const typename graph_traits<Graph>::vertex_descriptor& s)
        : Base(graph::get(color_t(), graph)),
        distance(graph::get(distance_t(), graph)) {
        graph::put(distance, s, 0);
    };
    void initialize_vertex(const typename graph_traits<Graph>::vertex_descriptor&, Graph&) {};
    typename graph::property_map<Graph, distance_t>::type distance;
};


TEST_P(DdsgGraphAlgorithm, BFSCorrectness) {
    using  Graph = GenerateBFSGraph<color_t,
        Properties<Property<distance_t, uint32_t>>,
        Properties<Property<edge_type_t, char>>>::type;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);

    auto colorPM = graph::get(color_t(), graph);
    auto distancePM = graph::get(distance_t(), graph);
    using BFSVisitor = CorrectnessBFSVisitor<Graph, property_map<Graph, color_t>::type>;
    auto vRange = vertices(graph);
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
        graph::put(colorPM, *vIt, 0);
        graph::put(distancePM, *vIt, m_numOfNodes);
    };
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt)
        breadth_first_search(graph, *vIt, colorPM, BFSVisitor(graph,*vIt));

    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
        EXPECT_EQ(2, graph::get(colorPM, *vIt));
        EXPECT_LT(graph::get(distancePM, *vIt), m_numOfNodes);
    };
};

template <typename Graph, typename PropertyMap>
struct SpeedBFSVisitor
    : graph::DefaultBFSVisitor<Graph, typename property_map<Graph, color_t>::type> {
    using Base = graph::DefaultBFSVisitor<Graph, typename property_map<Graph, color_t>::type>;
    SpeedBFSVisitor(Graph& graph)
        : Base(graph::get(color_t(), graph)) {};
    void initialize_vertex(const typename graph_traits<Graph>::vertex_descriptor&, Graph&) {};
};

TEST_P(DdsgGraphAlgorithm, BFSTraversalSpeed) {
    using  Graph = GenerateBFSGraph<color_t, Properties<>, Properties<>>::type;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);

    auto color = graph::get(color_t(), graph);
    using BFSVisitor = SpeedBFSVisitor<Graph, property_map<Graph, color_t>::type>;
    auto vRange = vertices(graph);
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt)
        graph::put(color, *vIt, 0);
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt)
        breadth_first_search(graph, *vIt, color, BFSVisitor(graph));
};


TEST_P(DdsgGraphAlgorithm, DijkstraOne2All) {
    using Graph = GenerateDijkstraGraph<predecessor_t, distance_t, weight_t,
        vertex_index_t, color_t, Properties<>, Properties< >> ::type;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);
    //    Graph graph;
    auto predecessor = graph::get(predecessor_t(), graph);
    auto distance = graph::get(distance_t(), graph);
    auto weight = graph::get(weight_t(), graph);
    auto vertex_index = graph::get(vertex_index_t(), graph);
    auto color = graph::get(color_t(), graph);
    dijkstra(graph, graph_traits<Graph>::vertex_descriptor(), predecessor, distance, weight, vertex_index, color);
};

TEST_P(DdsgGraphAlgorithm, BiDijkstra) {
    using Graph = GenerateBiDijkstraGraph<predecessor_t, predecessorB_t,
        distance_t, distanceB_t, weight_t, vertex_index_t, color_t, colorB_t,
        Properties<>, Properties<>> ::type;
    Graph graph;
    auto predecessorF = graph::get(predecessor_t(), graph);
    auto predecessorB = graph::get(predecessorB_t(), graph);
    auto distanceF = graph::get(distance_t(), graph);
    auto distanceB = graph::get(distanceB_t(), graph);
    auto weight = graph::get(weight_t(), graph);
    auto vertex_index = graph::get(vertex_index_t(), graph);
    auto colorF = graph::get(color_t(), graph);
    auto colorB = graph::get(color_t(), graph);
    bidirectional_dijkstra(graph, graph_traits<Graph>::vertex_descriptor(),
        graph_traits<Graph>::vertex_descriptor(),predecessorF, predecessorB,
        distanceF, distanceB, weight, vertex_index, colorF,colorB);
};


INSTANTIATE_TEST_CASE_P(CommandLine, DdsgGraphAlgorithm,
    ::testing::Values("nld.ddsg","bel.ddsg"));

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc >= 2) globalPathToFiles = argv[1]; 
    else {
        cerr << "Path to the folder with .ddsg graphs is required" << endl;
        return 1;
    };

    return RUN_ALL_TESTS();
};
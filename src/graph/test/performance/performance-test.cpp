#include <type_traits>
#include <utility>
#include <cassert>
#include <queue>
#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <string>
#include <boost/graph/graph_concepts.hpp>
#include <graph/static_graph.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/breadth_first_search.hpp>
#include <graph/dijkstra.hpp>
#include <graph/bidirectional_dijkstra.hpp>
#include <graph/io.hpp>

#include <util/statistics.h>
#include <test.h>
#include <generator.hpp>

using namespace std;
using namespace graph;
using namespace util::statistics;

struct distance_t {};
struct distanceB_t {};
struct color_t {};
struct colorB_t {};
struct edge_type_t {};
struct predecessor_t {};
struct predecessorB_t {};
struct weight_t {};

std::string baseFileName(const std::string& path) {
    std::string str = path.substr(path.find_last_of("/\\") + 1);
    std::string::size_type const p(str.find_last_of('.'));
    std::string base = str.substr(0, p);
    return base;
}

char* globalPathToFiles = nullptr;

class DdsgGraphAlgorithm : public ::testing::TestWithParam<const char*> {
protected:
    DdsgGraphAlgorithm() 
        :m_ddsgVecBackInserter(m_ddsgVec), m_path(globalPathToFiles),
        m_baseName(baseFileName(GetParam())) {};
    virtual void SetUp() {
        if (read_ddsg<Property<weight_t, uint32_t>>(m_ddsgVecBackInserter,
            m_numOfNodes, m_numOfEdges, (m_path + "/" + GetParam()).c_str()))
            FAIL();        
        std::sort(m_ddsgVec.begin(), m_ddsgVec.end(),
            [&](DdsgVecType::value_type left, DdsgVecType::value_type right) {
            return left.first.first < right.first.first;
        });
        stringstream ss;
        ss << m_path << "/" << m_baseName << "/" << m_baseName<<".sources";
        ifstream input(ss.str());
        if (!input.is_open()) {
            cerr << "Could not open respective test file: " << ss.str() << endl;
            FAIL();
        };
        size_t src;
        while (input >> src) m_sources.push_back(src);
        input.close();
    };

    using DdsgVecType = std::vector<std::pair<std::pair<size_t,size_t>,Property<weight_t, uint32_t>>>;
    DdsgVecType m_ddsgVec;
    back_insert_iterator<DdsgVecType> m_ddsgVecBackInserter;
    string m_path;
    string m_baseName;
    size_t m_numOfNodes;
    size_t m_numOfEdges;
    std::vector<size_t> m_sources;

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
        graph::put(colorPM, *vIt, 
            property_traits<property_map<Graph,color_t>::type>::value_type(0));
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
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt)
        graph::put(color, *vIt, 0);

    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt)
        breadth_first_search(graph, *vIt, color, BFSVisitor(graph));
    end = std::chrono::high_resolution_clock::now();

    BFSStatistics bfsStatistics(m_baseName, Algorithm::bfs, Phase::query, Metric::time, 
        m_numOfNodes, m_numOfEdges, chrono::duration_cast<chrono::milliseconds>(end - start).count(), 0, false);
    cout << bfsStatistics << endl;
};


TEST_P(DdsgGraphAlgorithm, DijkstraOne2All) {
    using Graph = GenerateDijkstraGraph<predecessor_t, distance_t, weight_t,
        vertex_index_t, color_t, Properties<>, Properties< >> ::type;
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);
    auto predecessor = graph::get(predecessor_t(), graph);
    auto distance = graph::get(distance_t(), graph);
    auto weight = graph::get(weight_t(), graph);
    auto vertex_index = graph::get(vertex_index_t(), graph);
    auto color = graph::get(color_t(), graph);
    ifstream verificationFile;
    graph::DefaultDijkstraVisitor<Graph> visitor;
    for (size_t src:m_sources) {
//        cout << "Testing source " << src << endl;
        start = std::chrono::high_resolution_clock::now();
        dijkstra(graph, graph_traits<Graph>::vertex_descriptor(src), predecessor,
            distance, weight, vertex_index, color, visitor);
        end = std::chrono::high_resolution_clock::now();
        DijkstraOneToAllSPStatistics statistics(
            GeneralStatistics(m_baseName, Algorithm::dijkstra, Phase::query, Metric::time,
                m_numOfNodes, m_numOfEdges,
                chrono::duration_cast<chrono::milliseconds>(end - start).count(), 0, false),
            src);
        cout << statistics << endl;
        stringstream ss;
        ss << m_path << "/" << m_baseName << "/" << m_baseName << "_" << src << ".sssp";
        verificationFile.open(ss.str());
        if (!verificationFile.is_open()) {
            cerr << "Verification file " << ss.str() << " for the source " << src << " is not found." << endl;
            FAIL();
        };
        size_t file_src, file_dist;
        verificationFile >> file_src;
        if (src != file_src) {
            cerr << "Source in the file is different from the expected";
            FAIL();
        }
        auto vRange = vertices(graph);
        for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
            verificationFile >> file_src >> file_dist;
            EXPECT_EQ(file_src, *vIt);
            EXPECT_EQ(file_dist, get(distance, *vIt));
        };
        verificationFile.close();
    };
};

TEST_P(DdsgGraphAlgorithm, BiDijkstra) {
    using Graph = GenerateBiDijkstraGraph<predecessor_t, predecessorB_t,
        distance_t, distanceB_t, weight_t, vertex_index_t, color_t, colorB_t,
        Properties<>, Properties<>> ::type;
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);
    auto predecessorF = graph::get(predecessor_t(), graph);
    auto predecessorB = graph::get(predecessorB_t(), graph);
    auto distanceF = graph::get(distance_t(), graph);
    auto distanceB = graph::get(distanceB_t(), graph);
    auto weight = graph::get(weight_t(), graph);
    auto vertex_index = graph::get(vertex_index_t(), graph);
    auto colorF = graph::get(color_t(), graph);
    auto colorB = graph::get(colorB_t(), graph);
    ifstream verificationFile;
    stringstream ss;
    ss << m_path << "/" << m_baseName << "/" << m_baseName << ".ppsp";
    verificationFile.open(ss.str());
    size_t src, tgt, distance;
    if (!verificationFile.is_open()) {
        cerr << "Verification file " << ss.str() <<" is not found." << endl;
        FAIL();
    };
    DefaultDijkstraVisitor<Graph> visitorF;
    DefaultDijkstraVisitor<Graph> visitorB;    
    while (verificationFile>>src>>tgt>>distance) {
//        cout << "Running BiDijkstra from " << src << " to " << tgt << endl;
        start = std::chrono::high_resolution_clock::now();
        bidirectional_dijkstra(graph, graph_traits<Graph>::vertex_descriptor(src),
        graph_traits<Graph>::vertex_descriptor(tgt),predecessorF, predecessorB,
        distanceF, distanceB, weight, vertex_index, colorF,colorB, visitorF, visitorB);
        end = std::chrono::high_resolution_clock::now();
        DijkstraSSSPStatistics statistics(
            GeneralStatistics(m_baseName, Algorithm::biDijkstra, Phase::query, Metric::time,
                m_numOfNodes, m_numOfEdges, chrono::duration_cast<chrono::milliseconds>(end - start).count(), 0, false),
            src,tgt,distance);
        cout << statistics << endl;
        EXPECT_EQ(distance, get(distanceF, tgt));
    }
    verificationFile.close();
};


INSTANTIATE_TEST_CASE_P(CommandLine, DdsgGraphAlgorithm,
    ::testing::Values("arc.ddsg","bel.ddsg"));

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc >= 2) globalPathToFiles = argv[1]; 
    else {
        cerr << "Path to the folder with .ddsg graphs is required" << endl;
        return 1;
    };

    return RUN_ALL_TESTS();
};
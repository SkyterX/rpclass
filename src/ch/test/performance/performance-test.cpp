#include <type_traits>
#include <utility>
#include <cassert>
#include <queue>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <gtest/gtest.h>
#include <graph/io.hpp>
#include <ch/contraction_hierarchy.hpp>
#include <test.h>

#include <gtest/gtest.h>

using namespace std;
using namespace graph;
using namespace ch;
using namespace util::statistics;

struct distanceF_t {};
struct distanceB_t {};
struct colorF_t {};
struct colorB_t {};
struct predecessorF_t {};
struct predecessorB_t {};
struct weight_t {};
struct unpack_t {};
struct vertex_order_t {};
struct direction_t {};

std::string baseFileName(const std::string& path) {
    std::string str = path.substr(path.find_last_of("/\\") + 1);
    std::string::size_type const p(str.find_last_of('.'));
    std::string base = str.substr(0, p);
    return base;
}

char* globalPathToFiles = nullptr;

class DdsgGraphAlgorithm : public ::testing::TestWithParam<tuple<const char*,int, bool>> {
protected:
    DdsgGraphAlgorithm()
        :m_ddsgVecBackInserter(m_ddsgVec), m_path(globalPathToFiles),
        m_fileName(get<0>(GetParam())), m_baseName(baseFileName(m_fileName)),
        m_numSteps(get<1>(GetParam())),m_stalling(get<2>(GetParam())) {};
    virtual void SetUp() {
        if (read_ddsg<Property<weight_t, uint32_t>, Property<direction_t, DirectionBit>>(
                m_ddsgVecBackInserter, m_numOfNodes, m_numOfEdges,
                (m_path + "/" + m_fileName).c_str()))
            FAIL();
        std::sort(m_ddsgVec.begin(), m_ddsgVec.end(),
            [&](DdsgVecType::value_type left, DdsgVecType::value_type right) {
            return left.first.first < right.first.first;
        });
        m_statistics.open("statistics", std::ofstream::out | std::ofstream::app);
    };
    virtual void TearDown() {
        m_statistics.close();
    }
    using DdsgVecType = std::vector<std::pair<std::pair<size_t, size_t>, 
        Properties<Property<weight_t, uint32_t>,Property<direction_t,DirectionBit>>>>;
    using N = integral_constant<size_t, 8>;
    DdsgVecType m_ddsgVec;
    back_insert_iterator<DdsgVecType> m_ddsgVecBackInserter;
    string m_path;
    string m_fileName;
    string m_baseName;
    size_t m_numSteps;
    bool m_stalling;
    size_t m_numOfNodes;
    size_t m_numOfEdges;
    ofstream m_statistics;
    
};

TEST_P(DdsgGraphAlgorithm, CH) {
    using Graph = GenerateCHGraph<predecessorF_t, predecessorB_t, distanceF_t, distanceB_t, weight_t,
        vertex_index_t, colorF_t, colorB_t, unpack_t, vertex_order_t, direction_t,
        Properties<>, Properties<>, Properties<>> ::type;
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);
    auto predecessorF = graph::get(predecessorF_t(), graph);
	auto predecessorB = graph::get(predecessorB_t(), graph);
    auto distanceF = graph::get(distanceF_t(), graph);
    auto distanceB = graph::get(distanceB_t(), graph);
    auto weight = graph::get(weight_t(), graph);
    auto vertex_index = graph::get(vertex_index_t(), graph);
    auto colorF = graph::get(colorF_t(), graph);
	auto colorB = graph::get(colorB_t(), graph);
    auto unpack = graph::get(unpack_t(), graph);
    auto order = graph::get(vertex_order_t(), graph);
    auto direction = graph::get(direction_t(), graph);
    stringstream ss;
    
    start = std::chrono::high_resolution_clock::now();
    ch_preprocess<Graph>(graph, predecessorF, distanceF, weight, vertex_index,
        colorF, unpack, order, direction, m_numSteps,
		VertexDegreeOrderStrategy<Graph>(graph));
    end = std::chrono::high_resolution_clock::now();
    CHMetricStatistics statistics(
        GeneralStatistics(m_baseName, Algorithm::CH, Phase::metric, Metric::time,
            m_numOfNodes, m_numOfEdges,
            chrono::duration_cast<chrono::milliseconds>(end - start).count(), 0),
        m_numSteps, CHPriority::shortcut);
    m_statistics << statistics << endl;


    ifstream verificationFile;
    ss.str(string());
    ss << m_path << "/" << m_baseName << "/" << m_baseName << ".ppsp";
    verificationFile.open(ss.str());

    if (!verificationFile.is_open()) {
        cerr << "Verification file " << ss.str() << " is not found." << endl;
        FAIL();
    };
    size_t src, tgt, dis;

    DefaultCHVisitor<Graph> visitor;
    while (verificationFile >> src >> tgt >> dis) {
//		if (src+1 != 8 || tgt+1 != 2) continue;
        cout << "Running CH query from " << src+1 << " to " << tgt+1 << endl;
        start = std::chrono::high_resolution_clock::now();
        ch_query(graph,
            graph_traits<Graph>::vertex_descriptor(src),
            graph_traits<Graph>::vertex_descriptor(tgt),
            predecessorF, predecessorB, distanceF, distanceB, weight, vertex_index, colorF, colorB, unpack, order, direction,visitor);
        end = std::chrono::high_resolution_clock::now();
        CHQueryStatistic statistics(
            CHMetricStatistics(
                GeneralStatistics(m_baseName, Algorithm::CH, Phase::query, Metric::time,
                    m_numOfNodes, m_numOfEdges,
                    chrono::duration_cast<chrono::milliseconds>(end - start).count(), 0),
                m_numSteps, CHPriority::shortcut), 
            src, tgt, get(distanceF, tgt), m_stalling
            );
        m_statistics << statistics << endl;
        EXPECT_EQ(dis, get(distanceF, tgt));
//		break;
    }    
    verificationFile.close();
};


INSTANTIATE_TEST_CASE_P(CommandLine, DdsgGraphAlgorithm,
    ::testing::Combine(::testing::Values("rome99.ddsg"), ::testing::Values(1500), ::testing::Values(false)));

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc >= 2) globalPathToFiles = argv[1];
    else {
        cerr << "Path to the folder with .ddsg graphs is required" << endl;
        return 1;
    };

    return RUN_ALL_TESTS();
};
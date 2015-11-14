#include <type_traits>
#include <utility>
#include <cassert>
#include <queue>
#include <vector>
#include <string>

#include <gtest/gtest.h>
#include <graph/io.hpp>
#include <ch/contraction_hierarchy.hpp>


#include <gtest/gtest.h>

using namespace std;
using namespace graph;
using namespace ch;

struct distance_t {};
struct color_t {};
struct predecessor_t {};
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

class DdsgGraphAlgorithm : public ::testing::TestWithParam<tuple<const char*, double>> {
protected:
    DdsgGraphAlgorithm()
        :m_ddsgVecBackInserter(m_ddsgVec), m_path(globalPathToFiles),
        m_fileName(get<0>(GetParam())),
        m_baseName(baseFileName(m_fileName)),
        m_filter(get<1>(GetParam())) {};
    virtual void SetUp() {
        if (read_ddsg<Property<weight_t, uint32_t>>(m_ddsgVecBackInserter,
            m_numOfNodes, m_numOfEdges, (m_path + "/" + m_fileName).c_str()))
            FAIL();
        std::sort(m_ddsgVec.begin(), m_ddsgVec.end(),
            [&](DdsgVecType::value_type left, DdsgVecType::value_type right) {
            return left.first.first < right.first.first;
        });
    };

    using DdsgVecType = std::vector<std::pair<std::pair<size_t, size_t>, Property<weight_t, uint32_t>>>;
    using N = integral_constant<size_t, 8>;
    DdsgVecType m_ddsgVec;
    back_insert_iterator<DdsgVecType> m_ddsgVecBackInserter;
    string m_path;
    string m_fileName;
    string m_baseName;
    double m_filter;
    size_t m_numOfNodes;
    size_t m_numOfEdges;
};

TEST_P(DdsgGraphAlgorithm, CH) {
    using Graph = GenerateCHGraph<predecessor_t, distance_t, weight_t,
        vertex_index_t, color_t, unpack_t, vertex_order_t, direction_t,
        Properties<>, Properties<>, Properties<>> ::type;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);
    auto predecessor = graph::get(predecessor_t(), graph);
    auto distance = graph::get(distance_t(), graph);
    auto weight = graph::get(weight_t(), graph);
    auto vertex_index = graph::get(vertex_index_t(), graph);
    auto color = graph::get(color_t(), graph);
    auto unpack = graph::get(unpack_t(), graph);
    auto order = graph::get(vertex_order_t(), graph);
    auto direction = graph::get(direction_t(), graph);
    stringstream ss;
    
    ch_preprocess<Graph>(graph, predecessor, distance, weight, vertex_index,
        color, unpack, order, direction);

    ifstream verificationFile;
    ss.str(string());
    ss << m_path << "/" << m_baseName << "/" << m_baseName << ".ppsp";
    verificationFile.open(ss.str());

    if (!verificationFile.is_open()) {
        cerr << "Verification file " << ss.str() << " is not found." << endl;
        FAIL();
    };
    size_t src, tgt, dis;
    while (verificationFile >> src >> tgt >> dis) {
        cout << "Running CH query from " << src << " to " << tgt << endl;
        ch_query(graph,
            graph_traits<Graph>::vertex_descriptor(src),
            graph_traits<Graph>::vertex_descriptor(tgt),
            predecessor, distance, weight, vertex_index,
            color, unpack, order, direction);
        EXPECT_EQ(dis, get(distance, tgt));
    }
    verificationFile.close();
};


INSTANTIATE_TEST_CASE_P(CommandLine, DdsgGraphAlgorithm,
    ::testing::Combine(::testing::Values("deu.ddsg"), ::testing::Values(0.0)));

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc >= 2) globalPathToFiles = argv[1];
    else {
        cerr << "Path to the folder with .ddsg graphs is required" << endl;
        return 1;
    };

    return RUN_ALL_TESTS();
};
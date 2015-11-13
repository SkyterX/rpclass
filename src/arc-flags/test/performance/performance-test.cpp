#include <type_traits>
#include <utility>
#include <cassert>
#include <queue>
#include <vector>
#include <string>
#include <type_traits>
#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/io.hpp>
#include <arc-flags/arc-flags.hpp>
#include <arc-flags/bidirectionalArcflags.hpp>
#include <fstream>

using namespace std;
using namespace graph;
using namespace arcflags;

struct distance_t {};
struct distanceB_t{};
struct color_t {};
struct colorB_t{};
struct predecessor_t {};
struct predecessorB_t{};
struct weight_t {};
struct partition_t {};
struct arc_flags_t {};
struct arc_flagsB_t {};


std::string baseFileName(const std::string& path) {
    std::string str = path.substr(path.find_last_of("/\\") + 1);
    std::string::size_type const p(str.find_last_of('.'));
    std::string base = str.substr(0, p);
    return base;
}

char* globalPathToFiles = nullptr;

class DdsgGraphAlgorithm : public ::testing::TestWithParam<tuple<const char*,double>> {
protected:
    DdsgGraphAlgorithm()
        :m_ddsgVecBackInserter(m_ddsgVec), m_path(globalPathToFiles),
        m_fileName(get<0>(GetParam())),
        m_baseName(baseFileName(m_fileName)),
        m_filter(get<1>(GetParam())){};
    virtual void SetUp() {
        if (read_ddsg<Property<weight_t, uint32_t>>(m_ddsgVecBackInserter,
            m_numOfNodes, m_numOfEdges, (m_path + "/" + m_fileName).c_str()))
            FAIL();
        std::stable_sort(m_ddsgVec.begin(), m_ddsgVec.end(),
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

TEST_P(DdsgGraphAlgorithm, ArcFlags) {
    using Graph = GenerateArcFlagsGraph<predecessor_t, distance_t, weight_t,
        vertex_index_t, color_t, arc_flags_t, partition_t, N::value,
        Properties<>, Properties< >> ::type;
	const bool ArcFlagsSavingEnabled = false;
    Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);    
    auto predecessor = graph::get(predecessor_t(), graph);
    auto distance = graph::get(distance_t(), graph);
    auto weight = graph::get(weight_t(), graph);
    auto vertex_index = graph::get(vertex_index_t(), graph);
    auto color = graph::get(color_t(), graph);
    auto partition = graph::get(partition_t(), graph);
    auto arc_flags = graph::get(arc_flags_t(), graph);
    stringstream ss;    

	cout << "Reading partition..." << endl;
    ss << m_path << "/" << m_baseName << "/tmppartition" << N::value;    
    if (read_partitioning<N::value, partition_t>(graph, ss.str().c_str())) {
        FAIL();
    };    

	cout << "Trying to laod arc-flags from file..." << endl;
	ss.str(string());
	ss << m_path << "/" << m_baseName << "/arcflags" << N::value;
	if (!ArcFlagsSavingEnabled || read_arcflags<N::value>(graph, arc_flags, ss.str().c_str())) {
		if (ArcFlagsSavingEnabled)
			cout << "No saved arc-flags found." << endl;
		else
			cout << "Arc-flags saving is disabled." << endl;
		cout << "Building arc-flags..." << endl;
		arcflags_preprocess<N::value>(graph, predecessor, distance, weight, vertex_index,
			color, partition, arc_flags, m_filter);
		
		if (ArcFlagsSavingEnabled) {
			cout << "Saving arc-flags..." << endl;
			if (save_arcflags<N::value>(graph, arc_flags, ss.str().c_str())) {
				FAIL();
			}
		}
	}
	cout << "Reducing arc-flags by " << m_filter * 100 << "%" << endl;
	arcflags_reduce_greedy<N::value>(graph, arc_flags, m_filter);

	cout << "Running queries..." << endl;
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
        cout << "Running ArcFlags query from " << src << " to " << tgt << endl;
        arcflags_query<N::value>(graph,
            graph_traits<Graph>::vertex_descriptor(src),
            graph_traits<Graph>::vertex_descriptor(tgt),
            predecessor, distance, weight, vertex_index,
            color, partition, arc_flags);
        EXPECT_EQ(dis, get(distance, tgt));
    }
    verificationFile.close();
};

TEST_P(DdsgGraphAlgorithm, BidirectionalArcFlags) {
	using Graph = GenerateBiArcFlagsGraph<predecessor_t, predecessorB_t, distance_t, distanceB_t, weight_t,
		vertex_index_t, color_t, colorB_t, arc_flags_t, arc_flagsB_t, partition_t, N::value,
		Properties<>, Properties< >> ::type;
	const bool ArcFlagsSavingEnabled = false;
	Graph graph(m_ddsgVec.begin(), m_ddsgVec.end(), m_numOfNodes, m_numOfEdges);
	auto predecessor = graph::get(predecessor_t(), graph);
	auto predecessorB = graph::get(predecessorB_t(), graph);
	auto distance = graph::get(distance_t(), graph);
	auto distanceB = graph::get(distanceB_t(), graph);
	auto weight = graph::get(weight_t(), graph);
	auto vertex_index = graph::get(vertex_index_t(), graph);
	auto color = graph::get(color_t(), graph);
	auto colorB = graph::get(colorB_t(), graph);
	auto partition = graph::get(partition_t(), graph);
	auto arc_flags = graph::get(arc_flags_t(), graph);
	auto arc_flagsB = graph::get(arc_flagsB_t(), graph);
	stringstream ss;

	cout << "Reading partition..." << endl;
	ss << m_path << "/" << m_baseName << "/tmppartition" << N::value;
	if (read_partitioning<N::value, partition_t>(graph, ss.str().c_str())) {
		FAIL();
	};

	cout << "Trying to laod arc-flags from file..." << endl;
	ss.str(string());
	ss << m_path << "/" << m_baseName << "/bidirectionalArcflags" << N::value;
	if (!ArcFlagsSavingEnabled || read_bidirectional_arcflags<N::value>(graph, arc_flags, arc_flagsB, ss.str().c_str())) {
		if (ArcFlagsSavingEnabled)
			cout << "No saved arc-flags found." << endl;
		else
			cout << "Arc-flags saving is disabled." << endl;
		cout << "Building arc-flags..." << endl;
		arcflags_preprocess<N::value>(graph, predecessor, predecessorB, distance, distanceB, weight, vertex_index,
			color, colorB, partition, arc_flags, arc_flagsB, m_filter);

		if (ArcFlagsSavingEnabled) {
			cout << "Saving arc-flags..." << endl;
			if (save_bidirectional_arcflags<N::value>(graph, arc_flags, arc_flagsB, ss.str().c_str())) {
				FAIL();
			}
		}
	}

	cout << "Running queries..." << endl;
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
		cout << "Running ArcFlags query from " << src << " to " << tgt << endl;
		arcflags_query<N::value>(graph,
			graph_traits<Graph>::vertex_descriptor(src),
			graph_traits<Graph>::vertex_descriptor(tgt),
			predecessor, distance, weight, vertex_index,
			color, partition, arc_flags);
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
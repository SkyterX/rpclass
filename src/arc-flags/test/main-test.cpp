#include <gtest/gtest.h>
#include <graph/static_graph.hpp>
#include <graph/io.hpp>
#include <arc-flags/arc-flags.hpp>

struct distance_t {};
struct color_t {};
struct edge_type_t {};
struct predecessor_t {};
struct weight_t {};
struct arc_flags_t {};
struct partition_t {};
using namespace arcflags;

TEST(GraphAlgorithms, ArcFlags) {
    constexpr size_t NumOfParts = 64;
    using namespace graph;
    using Graph = GenerateArcFlagsGraph<predecessor_t, distance_t, weight_t,
        vertex_index_t, color_t, arc_flags_t,partition_t, NumOfParts,
        Properties<>, Properties< >> ::type;
    Graph graph;
    auto predecessor = get(predecessor_t(), graph);
    auto distance = get(distance_t(), graph);
    auto weight = get(weight_t(), graph);
    auto vertex_index = get(vertex_bundle_t(), graph);
    auto color = get(color_t(), graph);
    auto partition = get(partition_t(), graph);
    auto arcflags = get(arc_flags_t(), graph);
    read_ddsg<weight_t>(graph, "PathToFile");
    read_partitioning<NumOfParts, partition_t>(graph, "PathToFile");
    arcflags_preprocess<NumOfParts>(graph, predecessor, distance, weight, vertex_index, color, partition, arcflags);
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
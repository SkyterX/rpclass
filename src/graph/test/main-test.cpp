#include <gtest/gtest.h>
#include <graph/static_graph.hpp>

using namespace std;
using namespace graph;

TEST(StaticGraph, Constructor) {
    StaticGraph staticGraph;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
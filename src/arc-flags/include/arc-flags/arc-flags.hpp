#pragma once

#include <graph/static_graph.hpp>

namespace arcflags {
template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
    typename IndexMapTag, typename ColorMapTag, typename ArcFlagsMapTag,
    typename PartitionMapTag, size_t N, 
    typename BundledVertexProperties, typename BundledEdgeProperties>
struct GenerateArcFlagsGraph {};

template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
    typename IndexMapTag, typename ColorMapTag, typename ArcFlagsMapTag,
    typename PartitionMapTag, size_t N, typename... P1s, typename... P2s>
struct GenerateArcFlagsGraph<PredecessorMapTag, DisanceMapTag, WeightMapTag, 
    IndexMapTag, ColorMapTag, ArcFlagsMapTag, PartitionMapTag, N, 
    graph::Properties<P1s...>, graph::Properties<P2s...>> {
    using type = graph::StaticGraph<
        graph::Properties<
            graph::Property<PredecessorMapTag, 
                typename graph::graph_traits<graph::StaticGraph<graph::Properties<>,graph::Properties<>>>::vertex_descriptor>,
            graph::Property<DisanceMapTag, uint32_t>,
            graph::Property<ColorMapTag, char>,
            graph::Property<PartitionMapTag, char>,
            P1s...>,
        graph::Properties<
            graph::Property<WeightMapTag, uint32_t>,
            graph::Property<ArcFlagsMapTag, uint64_t>,
            P2s...>>;
};
// read partitionining from a file
template <size_t NumOfParts, typename PartitionMapTag, typename Graph>
void read_partitioning(Graph& graph, const char* PathToFile) {};

// uses dijkstra, therefore should have at least all property maps used by dijkstra
template <size_t NumOfParts, typename Graph, typename PredecessorMap, typename DistanceMap, typename WeightMap,
    typename IndexMap, typename ColorMap, typename ArcFlagsMap, typename PartitionMap>
    void arcflags_preprocess(Graph& graph, PredecessorMap& predecessor, DistanceMap& distance,
        WeightMap& weight, IndexMap& index, ColorMap& color, PartitionMap& partition,
        ArcFlagsMap& arcflags, double filter = 0 ) {};
};
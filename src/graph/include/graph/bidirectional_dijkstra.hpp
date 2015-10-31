#pragma once
#include <graph/dijkstra.hpp>

namespace graph {


template <typename PredecessorMapFTag, typename PredecessorMapBTag,
    typename DisanceMapFTag, typename DisanceMapBTag, typename WeightMapTag,
    typename IndexMapTag, typename ColorMapFTag, typename ColorMapBTag,
    typename BundledVertexProperties, typename BundledEdgeProperties>
struct GenerateBiDijkstraGraph {};

template <typename PredecessorMapFTag, typename PredecessorMapBTag,
    typename DisanceMapFTag, typename DisanceMapBTag,
    typename WeightMapTag, typename IndexMapTag, typename ColorMapFTag,
    typename ColorMapBTag, typename... P1s, typename... P2s>
struct GenerateBiDijkstraGraph<PredecessorMapFTag, PredecessorMapBTag,
    DisanceMapFTag, DisanceMapBTag, WeightMapTag,
    IndexMapTag, ColorMapFTag, ColorMapBTag, Properties<P1s...>, Properties<P2s...>> {
    using type = StaticGraph<
        Properties<
            Property<PredecessorMapFTag, 
                typename graph_traits<StaticGraph<Properties<>,Properties<>>>::vertex_descriptor>,
            Property<PredecessorMapBTag,
                typename graph_traits<StaticGraph<Properties<>, Properties<>>>::vertex_descriptor>,
            Property<DisanceMapFTag, double>,
            Property<DisanceMapBTag, double>,
            Property<IndexMapTag, 
                typename graph_traits<StaticGraph<Properties<>, Properties<>>>::vertices_size_type>,
            Property<ColorMapFTag, char>,
            Property<ColorMapBTag, char>,
            P1s...>,
        Properties<
            Property<WeightMapTag, double>,
            P2s...>>;
};

template <class Graph, class PredecessorMapF, class PredecessorMapB,
class DistanceMapF, class DistanceMapB, 
class WeightMap, class IndexMap, class ColorMapF, class ColorMapB,
class DijkstraVisitorF = DefaultDijkstraVisitor<Graph>, 
class  DijkstraVisitorB = DefaultDijkstraVisitor<Graph>>
    void bidirectional_dijkstra(Graph& graph,
        const typename graph_traits<Graph>::vertex_descriptor& s,
        const typename graph_traits<Graph>::vertex_descriptor& t,
        PredecessorMapF& predecessorF, PredecessorMapB& predecessorB, 
        DistanceMapF& distanceF, DistanceMapB& distanceB, WeightMap& weight,
        IndexMap& index, ColorMapF& colorF, ColorMapB& colorB,
        DijkstraVisitorF visitorF = DijkstraVisitorF(),
        DijkstraVisitorF visitorB = DijkstraVisitorB()) {};
}
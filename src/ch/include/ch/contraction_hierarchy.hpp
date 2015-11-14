#pragma once

#include <graph/dijkstra.hpp>
#include <graph/dynamic_graph.hpp>
#include <boost/graph/two_bit_color_map.hpp>

namespace ch {

template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
    typename IndexMapTag, typename ColorMapTag, typename UnPackMapTag,
    typename VertexOrderMapTag, typename DirectionMapTag,
    typename VP, typename EP, typename GP>
struct GenerateCHGraph {};

template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
    typename IndexMapTag, typename ColorMapTag, typename UnPackMapTag,
    typename VertexOrderMapTag, typename DirectionMapTag,
    typename... P1s, typename... P2s, typename ... P3s>
struct GenerateCHGraph<PredecessorMapTag, DisanceMapTag, WeightMapTag,
    IndexMapTag, ColorMapTag, UnPackMapTag, VertexOrderMapTag, DirectionMapTag,
    graph::Properties<P1s...>, graph::Properties<P2s...>, graph::Properties<P3s... >> {
    using EmptyGraph = graph::DynamicGraph <
        graph::Properties<>, graph::Properties<>, graph::Properties<> >;

    using vertex_descriptor =
        typename graph::graph_traits<EmptyGraph>::vertex_descriptor;
    using vertices_size_type =
        typename graph::graph_traits<EmptyGraph>::vertices_size_type;

    using type = graph::DynamicGraph <
        graph::Properties<
            graph::Property<PredecessorMapTag, vertex_descriptor>,
            graph::Property<DisanceMapTag, uint32_t>,
            graph::Property<ColorMapTag, boost::two_bit_color_type>,
            graph::Property<VertexOrderMapTag, vertices_size_type>,            
            P1s...>,
        graph::Properties<
            graph::Property<WeightMapTag, uint32_t>,
            graph::Property<UnPackMapTag, vertex_descriptor>,
            graph::Property<DirectionMapTag, char>,
            P2s...>,
        graph::Properties<P3s...>
    >;
};

template <typename Graph>
class DefaultOrderStrategy {
public:
    //DefaultOrderStrategy(DefaultOrderStrategy&& other){
    //    *this(other); 
    //};
    typename graph::graph_traits<Graph>::vertex_descriptor
        next(Graph& graph) {};

};

// uses dijkstra, therefore should have at least all property maps used by dijkstra
template <typename Graph, typename PredecessorMap, typename DistanceMap,
    typename WeightMap, typename IndexMap, typename ColorMap, typename UnPackMap,
    typename VertexOrderMap, typename DirectionMap,
    typename OrderStrategy = DefaultOrderStrategy<Graph>>
    void ch_preprocess(Graph& graph, PredecessorMap& predecessor, DistanceMap& distance,
        WeightMap& weight, IndexMap& index, ColorMap& color, UnPackMap& unpack,
        VertexOrderMap& order, DirectionMap& direction,
        OrderStrategy&& strategy = OrderStrategy()) {};


template <typename Graph, typename PredecessorMap, typename DistanceMap,
    typename WeightMap, typename IndexMap, typename ColorMap, typename UnPackMap,
    typename VertexOrderMap, typename DirectionMap>
    void ch_query(Graph& graph, 
        const typename graph::graph_traits<Graph>::vertex_descriptor& s,
        const typename graph::graph_traits<Graph>::vertex_descriptor& t,
        PredecessorMap& predecessor, DistanceMap& distance,
        WeightMap& weight, IndexMap& index, ColorMap& color, UnPackMap& unpack,
        VertexOrderMap& order, DirectionMap& direction) {
        graph::dijkstra(graph, s, predecessor, distance, weight, index, color);
    };
};
#pragma once
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/static_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/two_bit_color_map.hpp>

namespace graph {

template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
    typename IndexMapTag, typename ColorMapTag, typename BundledVertexProperties,
    typename BundledEdgeProperties>
struct GenerateDijkstraGraph {};

template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
    typename IndexMapTag, typename ColorMapTag, typename... P1s, typename... P2s>
struct GenerateDijkstraGraph<PredecessorMapTag, DisanceMapTag, WeightMapTag, 
    IndexMapTag, ColorMapTag, Properties<P1s...>, Properties<P2s...>> {
    using type = StaticGraph<
        Properties<
            Property<PredecessorMapTag, 
                typename graph_traits<StaticGraph<Properties<>,Properties<>>>::vertex_descriptor>,
            Property<DisanceMapTag, uint32_t>,
            Property<ColorMapTag, boost::two_bit_color_type>,
            P1s...>,
        Properties<
            Property<WeightMapTag, uint32_t>,
            P2s...>>;
};

template <typename Graph> 
struct DefaultDijkstraVisitor {
    // This is invoked one each vertex of the graph when it is initialized.
    void initialize_vertex(const typename graph_traits<Graph>::vertex_descriptor&, Graph&) {};
    // This is invoked on a vertex as it is popped from the queue.
    // This happens immediately before examine_edge() is invoked on each of the out - edges of vertex u.
    void examine_vertex(const typename graph_traits<Graph>::vertex_descriptor&, Graph&) {};
    // This is invoked on every out - edge of each vertex after it is discovered.
    void examine_edge(const typename graph_traits<Graph>::edge_descriptor&, Graph&) {};
    // This is invoked when a vertex is encountered for the first time.
    void discover_vertex(const typename graph_traits<Graph>::vertex_descriptor&, Graph&) {};
    // Upon examination, if the following condition holds then the edge is relaxed(its distance is reduced), and this method is invoked.
    void edge_relaxed(const typename graph_traits<Graph>::edge_descriptor&, Graph&) {};
    // Upon examination, if the edge is not relaxed(see above) then this method is invoked.
    void edge_not_relaxed(const typename graph_traits<Graph>::edge_descriptor&, Graph&) {};
    // This invoked on a vertex after all of its out edges have been added to the search tree and
    // all of the adjacent vertices have been discovered(but before their out - edges have been examined).
    void finish_vertex(const typename graph_traits<Graph>::vertex_descriptor&, Graph&) {};
    // A predicate which is invoked on every out - edge of each vertex to check if the algorithm should relax it
    bool should_relax(const typename graph_traits<Graph>::edge_descriptor&, Graph&) { return true; };
    // A predicate  which is invoked after examine_vertex to check if the algorithm should continue
    bool should_continue() { return true; };
};

template <class Graph, class PredecessorMap, class DistanceMap, class WeightMap,
class IndexMap, class ColorMap, class DijkstraVisitor = DefaultDijkstraVisitor<Graph> >
    void dijkstra(Graph& graph,
        const typename graph_traits<Graph>::vertex_descriptor& s,
        PredecessorMap& predecessor, DistanceMap& distance, WeightMap& weight,
        IndexMap& index, ColorMap& color, DijkstraVisitor visitor = DefaultDijkstraVisitor<Graph>() ) {
    boost::dijkstra_shortest_paths(graph, s, predecessor, distance, weight, index,
        std::less<typename property_traits<DistanceMap>::value_type>(),
        boost::closed_plus<typename property_traits<DistanceMap>::value_type>(),
        std::numeric_limits<typename property_traits<DistanceMap>::value_type>::max(), 
        static_cast<typename property_traits<DistanceMap>::value_type>(0),
        boost::dijkstra_visitor<>(), color);

};

};
#pragma once
#include <queue>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/static_graph.hpp>


namespace graph {

    template <typename ColorMapTag, typename V, typename E>
    struct GenerateBFSGraph {};

    template <typename ColorMapTag, typename... P1s, typename... P2s>
    struct GenerateBFSGraph<ColorMapTag, Properties<P1s...>, Properties<P2s...>> {
        using type = StaticGraph<Properties<Property<ColorMapTag,char>,P1s...>,Properties<P2s...>>;
    };

template <typename Graph, typename ColorMap>
struct DefaultBFSVisitor {
    //is invoked on every vertex before the start of the search.
    void initialize_vertex(const typename graph_traits<Graph>::vertex_descriptor& v,
        Graph&) {
        graph::put(color, v, 0);
    };
    // is invoked in each vertex as it is removed from the queue.
    void examine_vertex(const typename graph_traits<Graph>::vertex_descriptor&,
        Graph&) {};
    //is invoked on every out - edge of each vertex immediately after the vertex
    // is removed from the queue.
    void examine_edge(const typename graph_traits<Graph>::edge_descriptor&,
        Graph& g) {};
    // is invoked(in addition to examine_edge()) if the edge is a tree edge.
    // The target vertex of edge e is discovered at this time.
    void tree_edge(const typename graph_traits<Graph>::edge_descriptor&,
        Graph& g) {};
    // is invoked the first time the algorithm encounters vertex u.
    // All vertices closer to the source vertex have been discovered, 
    // and vertices further from the source have not yet been discovered.
    void discover_vertex(const typename graph_traits<Graph>::vertex_descriptor&,
        Graph&) {};
    //is invoked(in addition to examine_edge()) if the edge is not a tree edge.
    void non_tree_edge(const typename graph_traits<Graph>::edge_descriptor&,
        Graph&) {};
    // is invoked(in addition to non_tree_edge()) if the target vertex is colored
    // gray at the time of examination.The color gray indicates that the vertex
    // is currently in the queue.
    void gray_target(const typename graph_traits<Graph>::edge_descriptor&,
        Graph&) {};
    // is invoked(in addition to non_tree_edge()) if the target vertex is colored
    // black at the time of examination.The color black indicates that the vertex
    // is no longer in the queue.
    void black_target(const typename graph_traits<Graph>::edge_descriptor&,
        Graph&) {};
    // is invoked after all of the out edges of u have been examined and all of
    // the adjacent vertices have been discovered.    
    void finish_vertex(const typename graph_traits<Graph>::vertex_descriptor&,
        Graph&) {};
    DefaultBFSVisitor(ColorMap& color) :color(color) {};
    DefaultBFSVisitor(ColorMap&& color) :color(color) {};
    ColorMap color;
};

template <class Graph, class ColorMap, 
    class BFSVisitor = DefaultBFSVisitor<Graph, ColorMap>>
    void breadth_first_search(Graph& graph,
        const typename graph_traits<Graph>::vertex_descriptor& s,
        ColorMap& color, BFSVisitor visitor = BFSVisitor{}) {
    std::queue<typename graph_traits<Graph>::vertex_descriptor> vQueue;
    auto vRange = vertices(graph);
    for (auto vIt = vRange.first; vIt != vRange.second; ++vIt) {
        if (graph::get(color, *vIt) == 0) {
            auto start = *vIt;
            vQueue.push(start);
            while (!vQueue.empty()) {
                auto src = vQueue.front();
                visitor.examine_vertex(src, graph);                    
                auto outRange = out_edges(src, graph);
                graph::put(color, src, 2);
                for (auto outIt = outRange.first; outIt != outRange.second; ++outIt) {
                    auto e = *outIt;
                    auto tgt = target(e, graph);
                    visitor.examine_edge(e, graph);
                    if (graph::get(color, tgt) == 0) {
                        visitor.discover_vertex(tgt, graph);
                        visitor.tree_edge(e, graph);
                        graph::put(color, tgt, 1);
                        vQueue.push(tgt);
                    }
                    else
                        if (graph::get(color, tgt) == 1) visitor.gray_target(e, graph);
                        else visitor.black_target(e,graph);
                        
                };
                visitor.finish_vertex(src,graph);
                vQueue.pop();
            }
        }
    };
};

}
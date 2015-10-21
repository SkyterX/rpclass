#pragma once
#include <type_traits>
namespace graph{




// traversal category tags

struct incidence_graph_tag { };
struct adjacency_graph_tag { };
struct bidirectional_graph_tag : virtual incidence_graph_tag { };
struct vertex_list_graph_tag { };

// directed category tags
struct directed_tag { };
struct undirected_tag { };
struct bidirectional_tag : directed_tag { };

// parallel edges tags
struct allow_parallel_edge_tag { };
struct disallow_parallel_edge_tag { };


namespace detail {


template < typename Graph >
class IsGraph {
    template < typename C, typename = typename C::traversal_category>
    static std::true_type check(const C&) {};
    static std::false_type check(...) {};
public:
    using type = decltype(check(std::declval<Graph>()));
    static const bool value = type::value;
};

};

//default graph traits specializations depending on traversal category of the graph

// default graph traits

template <typename Graph, typename TagDispatcher = typename Graph::traversal_category>
struct graph_traits {
    using vertex_descriptor      = typename Graph::vertex_descriptor;
    using edge_descriptor        = typename Graph::edge_descriptor;
    using directed_category      = typename Graph::directed_category;
    using edge_parallel_category = typename Graph::edge_parallel_category;
    using traversal_category     = typename Graph::traversal_category;
};

template <typename Graph>
struct graph_traits<Graph, incidence_graph_tag> : graph_traits<Graph,void> {
    using out_edge_iterator = typename Graph::out_edge_iterator;
    using degree_size_type  = typename Graph::degree_size_type;
};

template <typename Graph>
struct graph_traits<Graph, adjacency_graph_tag>: graph_traits<Graph,void> {
    using adjacency_iterator = typename Graph::adjacency_iterator;    
};

template <typename Graph>
struct graph_traits<Graph, vertex_list_graph_tag> : graph_traits<Graph, void> {
    using vertex_iterator    = typename Graph::vertex_iterator;
    using vertices_size_type = typename Graph::vertices_size_type;
};

template <typename Graph>
struct graph_traits<Graph, bidirectional_graph_tag> : graph_traits<Graph, incidence_graph_tag> {
    using in_edge_iterator = typename Graph::in_edge_iterator;    
};


};
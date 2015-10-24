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

template <typename Graph>
struct IsBidirectional
    :std::is_convertible<typename Graph::traversal_category, bidirectional_graph_tag> {};

template <typename Graph>
struct IsIncidence
    :std::is_convertible<typename Graph::traversal_category, incidence_graph_tag> {};

template <typename Graph>
struct IsAdjacency
    :std::is_convertible<typename Graph::traversal_category, adjacency_graph_tag> {};

template <typename Graph>
struct IsVertexList
    :std::is_convertible<typename Graph::traversal_category, vertex_list_graph_tag> {};

//default graph traits helper specializations depending on traversal category of the graph

template <typename Graph, typename Base = void, typename Tag = void, typename EnableIf = void>
struct graph_traits_helper:Base{};

template <typename Graph>
struct graph_traits_helper<Graph, void, void, std::enable_if_t<IsGraph<Graph>::value>> {
    using vertex_descriptor = typename Graph::vertex_descriptor;
    using edge_descriptor = typename Graph::edge_descriptor;
    using directed_category = typename Graph::directed_category;
    using edge_parallel_category = typename Graph::edge_parallel_category;
    using traversal_category = typename Graph::traversal_category;
    using vertices_size_type = typename Graph::vertices_size_type;
    using edges_size_type = typename Graph::edges_size_type;
};

template <typename Graph, typename Base>
struct graph_traits_helper<Graph, Base, bidirectional_tag, 
    std::enable_if_t<IsBidirectional<Graph>::value>>:Base {
    using in_edge_iterator = typename Graph::in_edge_iterator;
};

template <typename Graph, typename Base>
struct graph_traits_helper<Graph, Base, incidence_graph_tag,
    std::enable_if_t<IsIncidence<Graph>::value >> :Base {
    using out_edge_iterator = typename Graph::out_edge_iterator;
    using degree_size_type  = typename Graph::degree_size_type;
};

template <typename Graph, typename Base>
struct graph_traits_helper<Graph, Base, adjacency_graph_tag,
    std::enable_if_t<IsAdjacency<Graph>::value >> :Base {
    using adjacency_iterator = typename Graph::adjacency_iterator;
};

template <typename Graph, typename Base>
struct graph_traits_helper<Graph, Base, vertex_list_graph_tag,
    std::enable_if_t<IsVertexList<Graph>::value >> :Base {
    using vertex_iterator    = typename Graph::vertex_iterator;
};

template <typename Graph, typename ... Tags>
struct graph_traits_generator{};

template <typename Graph, typename Tag, typename ... Tags> 
struct graph_traits_generator<Graph, Tag, Tags...> {
    using type = graph_traits_helper<Graph, 
        typename graph_traits_generator<Graph, Tags...>::type,
        Tag>;
};
template <typename Graph>
struct graph_traits_generator<Graph> {
    using type = graph_traits_helper < Graph, void, void>;
};

};


// default graph traits

template <typename Graph> 
struct graph_traits
    : detail::graph_traits_generator<Graph, vertex_list_graph_tag, adjacency_graph_tag, 
        incidence_graph_tag, bidirectional_graph_tag>::type {};
};
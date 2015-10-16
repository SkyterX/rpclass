#pragma once
#include <type_traits>
namespace graph{


    namespace detail {
    template < typename Graph >
    class IsGraph {
        template < typename C, typename = typename C::vertex_descriptor>
        static std::true_type check(const C&) {};
        static std::false_type check(...) {};
    public:
        using type = decltype(check(std::declval<Graph>()));
        static const bool value = type::value;
    };
    };

template <typename Graph, typename EnableIf = void>
struct graph_traits;

// default graph traits
template <typename Graph>
struct graph_traits<Graph,std::enable_if_t<detail::IsGraph<Graph>::value>> {
    using vertex_descriptor      = typename Graph::vertex_descriptor ;
    using edge_descriptor        = typename Graph::edge_descriptor;
    using directed_category      = typename Graph::directed_category;
    using edge_parallel_category = typename Graph::edge_parallel_category;
    using traversal_category     = typename Graph::traversal_category;
};

};
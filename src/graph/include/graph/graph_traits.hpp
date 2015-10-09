#pragma once

namespace graph{

template <typename G>
struct graph_traits {
    using edges_size_type    = typename G::edges_size_type;
    using vertices_size_type = typename G::vertices_size_type;
    using degree_size_type   = typename G::degree_size_type;
    using vertex_descriptor  = typename G::vertex_descriptor;
    using adjacency_iterator = typename G::adjacency_iterator;
    using vertex_iterator    = typename G::vertex_iterator;
};

};
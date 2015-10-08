#pragma once

#include <vector>
#include <cinttypes>

using namespace std;

namespace graph {
template <typename Iterator>
class Vertex {
    Iterator begin;
};

class StaticGraph {

    class VertexIterator;
public:
    using edge_size_type = uint32_t;
    using vertices_size_type = uint32_t;
    using edges_size_type = uint32_t;
    using degree_size_type = uint16_t;
    using vertex_descriptor = vertices_size_type;
private:
    using EdgesVecType = std::vector<vertex_descriptor>;
public:
    using adjacency_iterator = EdgesVecType::const_iterator;
    using VertexType = Vertex<EdgesVecType::const_iterator>;
private:
    using VerticesVecType = std::vector<VertexType>;
public:
    class vertex_iterator {};
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    StaticGraph() {};
    template <class PairIterator>
    StaticGraph(PairIterator begin, PairIterator end,
        vertices_size_type n, edges_size_type m);

};

std::pair<StaticGraph::vertex_iterator, StaticGraph::vertex_iterator>
vertices(const StaticGraph& g);

std::pair<StaticGraph::adjacency_iterator, StaticGraph::adjacency_iterator>
adjacent_vertices(StaticGraph::vertex_descriptor u, const StaticGraph& g);

std::pair<StaticGraph::edge_descriptor, bool>
edge(StaticGraph::vertex_descriptor u, StaticGraph::vertex_descriptor v,
    const StaticGraph& g);

StaticGraph::vertex_descriptor
source(StaticGraph::edge_descriptor e, const StaticGraph& g);

StaticGraph::vertex_descriptor
target(StaticGraph::edge_descriptor e, const StaticGraph& g);

StaticGraph::degree_size_type
out_degree(StaticGraph::vertex_descriptor u, const StaticGraph& g);

StaticGraph::vertices_size_type
num_vertices(const StaticGraph& g);

StaticGraph::edges_size_type
num_edges(const StaticGraph& g);

}
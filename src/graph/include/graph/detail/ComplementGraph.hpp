#pragma once
#include <graph/properties.hpp>

namespace graph {
	template <typename Graph>
	class ComplementGraph {
	public:
		using InnerGraphType = Graph;

		using edge_size_type = typename Graph::edge_size_type;
		using vertices_size_type = typename Graph::vertices_size_type;
		using edges_size_type = typename Graph::edges_size_type;
		using degree_size_type = typename Graph::degree_size_type;
		using vertex_descriptor = typename Graph::vertex_descriptor;
		using directed_category = typename Graph::directed_category;
		using edge_parallel_category = typename Graph::edge_parallel_category;
		using traversal_category = typename Graph::traversal_category;
		using edge_descriptor = typename Graph::edge_descriptor;

		using VertexPropertyMapType = typename Graph::VertexPropertyMapType;
		using EdgePropertyMapType = typename Graph::EdgePropertyMapType;
		using vertex_bundled = typename Graph::vertex_bundled;
		using edge_bundled = typename Graph::edge_bundled;

		using in_edge_iterator = typename Graph::out_edge_iterator;
		using out_edge_iterator = typename Graph::in_edge_iterator;
		using adjacency_iterator = typename Graph::adjacency_iterator;
		using vertex_iterator = typename Graph::vertex_iterator;

		static vertex_descriptor null_vertex() {
			return Graph::null_vertex();
		};

		ComplementGraph(Graph& graph)
			: innerGraph(graph) {}

		Graph& innerGraph;
	};

	// PropertyMaps
#define ComplementGraphType ComplementGraph<Graph>

	template <typename Graph>
	struct property_map<ComplementGraphType, vertex_bundle_t> {
		using type = typename property_map_t<typename ComplementGraphType::InnerGraphType, vertex_bundle_t>;
	};

	template <typename Graph>
	struct property_map<ComplementGraphType, edge_bundle_t> {
		using type = typename property_map_t<typename ComplementGraphType::InnerGraphType, edge_bundle_t>;
	};

	template <typename Graph>
	inline typename property_map<ComplementGraphType, vertex_bundle_t>::type
	get(const vertex_bundle_t& bundle, ComplementGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}

	template <typename Graph>
	inline typename property_map<ComplementGraphType, edge_bundle_t>::type
	get(const edge_bundle_t& bundle, ComplementGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}

	// External functions

	template <typename Graph>
	inline std::pair<typename ComplementGraphType::vertex_iterator, typename ComplementGraphType::vertex_iterator>
	vertices(const ComplementGraphType& g) {
		return vertices(g.innerGraph);
	}

	template <typename Graph>
	inline std::pair<typename ComplementGraphType::adjacency_iterator, typename ComplementGraphType::adjacency_iterator>
	adjacent_vertices(typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return in_adjacent_vertices(u, g.innerGraph);
	}

	template <typename Graph>
	inline std::pair<typename ComplementGraphType::adjacency_iterator, typename ComplementGraphType::adjacency_iterator>
	in_adjacent_vertices(typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return adjacent_vertices(u, g.innerGraph);
	}

	template <typename Graph>
	inline typename ComplementGraphType::vertex_descriptor source(
		typename ComplementGraphType::edge_descriptor e, const ComplementGraphType& g) {
		return target(e, g.innerGraph);
	}

	template <typename Graph>
	inline typename ComplementGraphType::vertex_descriptor target(
		typename ComplementGraphType::edge_descriptor e, const ComplementGraphType& g) {
		return source(e, g.innerGraph);
	}

	template <typename Graph>
	inline std::pair<typename ComplementGraphType::out_edge_iterator, typename ComplementGraphType::out_edge_iterator>
	out_edges(typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return in_edges(u, g.innerGraph);
	}

	template <typename Graph>
	inline std::pair<typename ComplementGraphType::in_edge_iterator, typename ComplementGraphType::in_edge_iterator>
	in_edges(typename ComplementGraphType::vertex_descriptor v, const ComplementGraphType& g) {
		return out_edges(v, g.innerGraph);
	}

	template <typename Graph>
	inline typename ComplementGraphType::degree_size_type out_degree(
		typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return in_degree(u, g.innerGraph);
	}

	template <typename Graph>
	inline typename ComplementGraphType::degree_size_type in_degree(
		typename ComplementGraphType::vertex_descriptor v, const ComplementGraphType& g) {
		return out_degree(v, g.innerGraph);
	}

	template <typename Graph>
	inline typename ComplementGraphType::degree_size_type degree(
		typename ComplementGraphType::vertex_descriptor v, const ComplementGraphType& g) {
		return in_degree(v, g.innerGraph) + out_degree(v, g.innerGraph);
	}

	template <typename Graph>
	inline typename ComplementGraphType::vertices_size_type num_vertices(const ComplementGraphType& g) {
		return num_vertices(g.innerGraph);
	}

	template <typename Graph>
	inline typename ComplementGraphType::edges_size_type num_edges(const ComplementGraphType& g) {
		return num_edges(g.innerGraph);
	}
#undef ComplementGraphType
}

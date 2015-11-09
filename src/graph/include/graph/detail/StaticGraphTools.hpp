#pragma once

namespace graph {
#define Properties VertexProperties, EdgeProperties
	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::vertex_iterator, typename StaticGraph<Properties>::vertex_iterator>
	vertices(const StaticGraph<Properties>& g) {
		return std::make_pair(g.Vertices().begin(), g.Vertices().end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::adjacency_iterator, typename StaticGraph<Properties>::adjacency_iterator>
	adjacent_vertices(typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.OutAdjacencies(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::adjacency_iterator, typename StaticGraph<Properties>::adjacency_iterator>
	in_adjacent_vertices(typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.InAdjacencies(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::vertex_descriptor source(
		typename StaticGraph<Properties>::edge_descriptor e, const StaticGraph<Properties>&) {
		return e.source;
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::vertex_descriptor target(
		typename StaticGraph<Properties>::edge_descriptor e, const StaticGraph<Properties>&) {
		return e.target;
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::degree_size_type out_degree(
		typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		return g.OutAdjacencies(u).size();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::out_edge_iterator, typename StaticGraph<Properties>::out_edge_iterator>
	out_edges(typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.OutEdges(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::vertices_size_type num_vertices(const StaticGraph<Properties>& g) {
		return g.Vertices().size();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::edges_size_type num_edges(const StaticGraph<Properties>& g) {
		return g.EdgesCount();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::in_edge_iterator, typename StaticGraph<Properties>::in_edge_iterator>
	in_edges(typename StaticGraph<Properties>::vertex_descriptor v, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.InEdges(v);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::degree_size_type in_degree(
		typename StaticGraph<Properties>::vertex_descriptor v, const StaticGraph<Properties>& g) {
		return g.InAdjacencies(v).size();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::degree_size_type degree(
		typename StaticGraph<Properties>::vertex_descriptor v, const StaticGraph<Properties>& g) {
		return in_degree(v, g) + out_degree(v, g);
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::edge_descriptor, bool> edge(
		typename StaticGraph<Properties>::vertex_descriptor u,
		typename StaticGraph<Properties>::vertex_descriptor v,
		const StaticGraph<Properties>& graph) {
		using EdgeDescriptor = typename StaticGraph<Properties>::edge_descriptor;
		for (const auto& edge : graphUtil::Range(out_edges(u, graph))) {
			if (target(edge, graph) == v) return make_pair(edge, true);
		}
		return make_pair(EdgeDescriptor(), false);
	}
#undef Properties
}

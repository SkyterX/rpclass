#pragma once
#pragma once

namespace graph {
#define DynamicGraphTemplate template<typename VertexProperties, typename EdgeProperties>
#define DynamicGraphType DynamicGraph<VertexProperties, EdgeProperties>
	DynamicGraphTemplate
	inline std::pair<typename DynamicGraphType::vertex_iterator, typename DynamicGraphType::vertex_iterator>
	vertices(const DynamicGraphType& g) {
		auto vertexCollection = g.Vertices();
		return std::make_pair(vertexCollection.begin(), vertexCollection.end());
	}

	DynamicGraphTemplate
	inline std::pair<typename DynamicGraphType::adjacency_iterator, typename DynamicGraphType::adjacency_iterator>
	adjacent_vertices(typename DynamicGraphType::vertex_descriptor u, const DynamicGraphType& g) {
		auto edgesCollection = g.OutAdjacencies(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	DynamicGraphTemplate
	inline typename DynamicGraphType::vertex_descriptor source(
		typename DynamicGraphType::edge_descriptor e, const DynamicGraphType& g) {
		return e.source;
	}

	DynamicGraphTemplate
	inline typename DynamicGraphType::vertex_descriptor target(
		typename DynamicGraphType::edge_descriptor e, const DynamicGraphType& g) {
		return e.target;
	}

	DynamicGraphTemplate
	inline std::pair<typename DynamicGraphType::out_edge_iterator, typename DynamicGraphType::out_edge_iterator>
	out_edges(typename DynamicGraphType::vertex_descriptor u, DynamicGraphType& g) {
		auto edgesCollection = g.OutEdges(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());

	}

	DynamicGraphTemplate
	inline typename DynamicGraphType::degree_size_type out_degree(
		typename DynamicGraphType::vertex_descriptor u, const DynamicGraphType& g) {
		return g.Degree(u);
	}

	DynamicGraphTemplate
	inline typename DynamicGraphType::degree_size_type degree(
		typename DynamicGraphType::vertex_descriptor v, const DynamicGraphType& g) {
		return out_degree(v, g);
	}

	DynamicGraphTemplate
	inline typename DynamicGraphType::vertices_size_type num_vertices(const DynamicGraphType& g) {
		return g.Vertices().size();
	}

	DynamicGraphTemplate
	inline typename DynamicGraphType::edges_size_type num_edges(const DynamicGraphType& g) {
		return g.EdgesCount();
	}

	DynamicGraphTemplate
	inline std::pair<typename DynamicGraphType::edge_descriptor, bool> edge(
		typename DynamicGraphType::vertex_descriptor u,
		typename DynamicGraphType::vertex_descriptor v,
		const DynamicGraphType& graph) {
		using EdgeDescriptor = typename DynamicGraphType::edge_descriptor;
		for (const auto& edge : graphUtil::Range(out_edges(u, graph))) {
			if (target(edge, graph) == v) return make_pair(edge, true);
		}
		return std::make_pair(EdgeDescriptor(), false);
	}

	DynamicGraphTemplate
	inline std::pair<typename DynamicGraphType::edge_descriptor, bool> add_edge(
		typename DynamicGraphType::vertex_descriptor u,
		typename DynamicGraphType::vertex_descriptor v,
		const DynamicGraphType& graph) {
		return graph.AddEdge(u, v);
	}
	//
	//	DynamicGraphTemplate
	//		inline decltype(auto) remove_edge(
	//			typename DynamicGraphType::edge_descriptor e,
	//			const DynamicGraphType& graph) {
	//		return remove_edge(e, graph.innerGraph);
	//	}

#undef DynamicGraphType
#undef DynamicGraphTemplate
}

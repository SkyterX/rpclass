#pragma once
#include <graph/properties.hpp>

namespace graph
{
	template <typename Graph, typename VertexProperties, typename EdgeProperties, typename EnableIf = void>
	class ComplementGraphObfuscated;

	template <typename Graph, typename VertexProperties, typename EdgeProperties>
	class ComplementGraphObfuscated
			<Graph, VertexProperties, EdgeProperties,
			 std::enable_if_t<
				 detail::IsGraph<Graph>::value &&
				 std::is_same<VertexProperties, typename Graph::vertex_bundled>::value &&
				 std::is_same<EdgeProperties, typename Graph::edge_bundled>::value
			 >
			> {
	public:
		using InnerGraphType = Graph;

		//		using edge_size_type = typename Graph::edge_size_type;
		using vertices_size_type = typename Graph::vertices_size_type;
		using edges_size_type = typename Graph::edges_size_type;
		using degree_size_type = typename Graph::degree_size_type;
		using vertex_descriptor = typename Graph::vertex_descriptor;
		using directed_category = typename Graph::directed_category;
		using edge_parallel_category = typename Graph::edge_parallel_category;
		using traversal_category = typename Graph::traversal_category;
		using edge_descriptor = typename Graph::edge_descriptor;

		//		using VertexPropertyMapType = typename Graph::VertexPropertyMapType;
		//		using EdgePropertyMapType = typename Graph::EdgePropertyMapType;
		using vertex_bundled = typename Graph::vertex_bundled;
		using edge_bundled = typename Graph::edge_bundled;

		using in_edge_iterator = typename Graph::out_edge_iterator;
		using out_edge_iterator = typename Graph::in_edge_iterator;
		using adjacency_iterator = typename Graph::adjacency_iterator;
		using vertex_iterator = typename Graph::vertex_iterator;

		static vertex_descriptor null_vertex() {
			return Graph::null_vertex();
		};

		explicit ComplementGraphObfuscated(Graph& graph)
			: innerGraph(graph) {}

		operator Graph&() {
			return innerGraph;
		}

//		template<typename TargetGraph = typename Graph::InnerGraphType>
//		operator std::enable_if_t<std::is_class<TargetGraph>::value, TargetGraph>&() {
//			return static_cast<typename Graph::InnerGraphType>(innerGraph);
//		}

		Graph& innerGraph;
	};

	template <typename Graph>
	using ComplementGraph = ComplementGraphObfuscated<Graph, typename Graph::vertex_bundled, typename Graph::edge_bundled>;

	template <typename Graph>
	ComplementGraph<Graph> CreateComplementGraph(Graph& graph) {
		return ComplementGraph<Graph>(graph);
	}

	// PropertyMaps
#define ComplementGraphTemplate template<typename Graph, typename VertexProperties, typename EdgeProperties>
#define ComplementGraphType ComplementGraphObfuscated<Graph, VertexProperties, EdgeProperties>

	ComplementGraphTemplate
	struct property_map<ComplementGraphType, vertex_bundle_t> {
		using type = typename property_map<typename ComplementGraphType::InnerGraphType, vertex_bundle_t>::type;
	};

	ComplementGraphTemplate
	struct property_map<ComplementGraphType, edge_bundle_t> {
		using type = typename property_map<typename ComplementGraphType::InnerGraphType, edge_bundle_t>::type;
	};

	ComplementGraphTemplate
	struct property_map<ComplementGraphType, vertex_index_t> {
		using type = typename property_map<typename ComplementGraphType::InnerGraphType, vertex_index_t>::type;
	};

	ComplementGraphTemplate
	inline typename property_map<ComplementGraphType, vertex_bundle_t>::type
	get(const vertex_bundle_t& bundle, ComplementGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}

	ComplementGraphTemplate
	inline typename property_map<ComplementGraphType, edge_bundle_t>::type
	get(const edge_bundle_t& bundle, ComplementGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}

	ComplementGraphTemplate
	inline typename property_map<ComplementGraphType, vertex_index_t>::type
	get(const vertex_index_t& bundle, ComplementGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}


	// External functions

	ComplementGraphTemplate
	inline std::pair<typename ComplementGraphType::vertex_iterator, typename ComplementGraphType::vertex_iterator>
	vertices(const ComplementGraphType& g) {
		return vertices(g.innerGraph);
	}

	ComplementGraphTemplate
	inline std::pair<typename ComplementGraphType::adjacency_iterator, typename ComplementGraphType::adjacency_iterator>
	adjacent_vertices(typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return in_adjacent_vertices(u, g.innerGraph);
	}

	ComplementGraphTemplate
	inline std::pair<typename ComplementGraphType::adjacency_iterator, typename ComplementGraphType::adjacency_iterator>
	in_adjacent_vertices(typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return adjacent_vertices(u, g.innerGraph);
	}

	ComplementGraphTemplate
	inline typename ComplementGraphType::vertex_descriptor source(
		typename ComplementGraphType::edge_descriptor e, const ComplementGraphType& g) {
		return target(e, g.innerGraph);
	}

	ComplementGraphTemplate
	inline typename ComplementGraphType::vertex_descriptor target(
		typename ComplementGraphType::edge_descriptor e, const ComplementGraphType& g) {
		return source(e, g.innerGraph);
	}

	ComplementGraphTemplate
	inline std::pair<typename ComplementGraphType::out_edge_iterator, typename ComplementGraphType::out_edge_iterator>
	out_edges(typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return in_edges(u, g.innerGraph);
	}

	ComplementGraphTemplate
	inline std::pair<typename ComplementGraphType::in_edge_iterator, typename ComplementGraphType::in_edge_iterator>
	in_edges(typename ComplementGraphType::vertex_descriptor v, const ComplementGraphType& g) {
		return out_edges(v, g.innerGraph);
	}

	ComplementGraphTemplate
	inline typename ComplementGraphType::degree_size_type out_degree(
		typename ComplementGraphType::vertex_descriptor u, const ComplementGraphType& g) {
		return in_degree(u, g.innerGraph);
	}

	ComplementGraphTemplate
	inline typename ComplementGraphType::degree_size_type in_degree(
		typename ComplementGraphType::vertex_descriptor v, const ComplementGraphType& g) {
		return out_degree(v, g.innerGraph);
	}

	ComplementGraphTemplate
	inline typename ComplementGraphType::degree_size_type degree(
		typename ComplementGraphType::vertex_descriptor v, const ComplementGraphType& g) {
		return in_degree(v, g.innerGraph) + out_degree(v, g.innerGraph);
	}

	ComplementGraphTemplate
	inline typename ComplementGraphType::vertices_size_type num_vertices(const ComplementGraphType& g) {
		return num_vertices(g.innerGraph);
	}

	ComplementGraphTemplate
	inline typename ComplementGraphType::edges_size_type num_edges(const ComplementGraphType& g) {
		return num_edges(g.innerGraph);
	}

	ComplementGraphTemplate
	inline std::pair<typename ComplementGraphType::edge_descriptor, bool> edge(
		typename ComplementGraphType::vertex_descriptor u,
		typename ComplementGraphType::vertex_descriptor v,
		const ComplementGraphType& graph) {
		return edge(v, u, graph.innerGraph);
	}
#undef ComplementGraphType
#undef ComplementGraphTemplate
}

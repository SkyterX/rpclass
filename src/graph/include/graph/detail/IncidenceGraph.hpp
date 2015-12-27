#pragma once
#include <graph/properties.hpp>

namespace graph
{
	template <typename Graph, typename VertexProperties, typename EdgeProperties, typename EnableIf = void>
	class IncidenceGraphObfuscated;

	template <typename Graph, typename VertexProperties, typename EdgeProperties>
	class IncidenceGraphObfuscated
			<Graph, VertexProperties, EdgeProperties,
			 std::enable_if_t<
				 detail::IsGraph<Graph>::value &&
				 std::is_same<VertexProperties, typename Graph::vertex_bundled>::value &&
				 std::is_same<EdgeProperties, typename Graph::edge_bundled>::value
			 >
			> {
	public:
		using InnerGraphType = Graph;

		using vertices_size_type = typename Graph::vertices_size_type;
		using edges_size_type = typename Graph::edges_size_type;
		using degree_size_type = typename Graph::degree_size_type;
		using vertex_descriptor = typename Graph::vertex_descriptor;
		using directed_category = typename Graph::directed_category;
		using edge_parallel_category = typename Graph::edge_parallel_category;
		using traversal_category = typename Graph::traversal_category;
		using edge_descriptor = typename Graph::edge_descriptor;

		using vertex_bundled = typename Graph::vertex_bundled;
		using edge_bundled = typename Graph::edge_bundled;

		using in_edge_iterator = typename Graph::out_edge_iterator;
		using out_edge_iterator = typename Graph::out_edge_iterator;
		using adjacency_iterator = typename Graph::adjacency_iterator;
		using vertex_iterator = typename Graph::vertex_iterator;

		static vertex_descriptor null_vertex() {
			return Graph::null_vertex();
		};

		explicit IncidenceGraphObfuscated(Graph& graph)
			: innerGraph(graph) {}

		operator Graph&() {
			return innerGraph;
		}
//
//		template <typename TargetGraph = typename Graph::InnerGraphType>
//		operator std::enable_if_t<std::is_class<TargetGraph>::value, TargetGraph>&() {
//			return static_cast<typename Graph::InnerGraphType>(innerGraph);
//		}

		Graph& innerGraph;
	};

	template <typename Graph>
	using IncidenceGraph = IncidenceGraphObfuscated<Graph, typename Graph::vertex_bundled, typename Graph::edge_bundled>;

	template <typename Graph>
	IncidenceGraph<Graph> CreateIncidenceGraph(Graph& graph) {
		return IncidenceGraph<Graph>(graph);
	}

	// PropertyMaps
#define IncidenceGraphTemplate template<typename Graph, typename VertexProperties, typename EdgeProperties>
#define IncidenceGraphType IncidenceGraphObfuscated<Graph, VertexProperties, EdgeProperties>

	IncidenceGraphTemplate
	struct property_map<IncidenceGraphType, vertex_bundle_t> {
		using type = typename property_map<typename IncidenceGraphType::InnerGraphType, vertex_bundle_t>::type;
	};

	IncidenceGraphTemplate
	struct property_map<IncidenceGraphType, edge_bundle_t> {
		using type = typename property_map<typename IncidenceGraphType::InnerGraphType, edge_bundle_t>::type;
	};

	IncidenceGraphTemplate
	struct property_map<IncidenceGraphType, vertex_index_t> {
		using type = typename property_map<typename IncidenceGraphType::InnerGraphType, vertex_index_t>::type;
	};

	IncidenceGraphTemplate
	inline typename property_map<IncidenceGraphType, vertex_bundle_t>::type
	get(const vertex_bundle_t& bundle, IncidenceGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename property_map<IncidenceGraphType, edge_bundle_t>::type
	get(const edge_bundle_t& bundle, IncidenceGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename property_map<IncidenceGraphType, vertex_index_t>::type
	get(const vertex_index_t& bundle, IncidenceGraphType& graph) {
		return get(bundle, graph.innerGraph);
	}


	// External functions

	IncidenceGraphTemplate
	inline std::pair<typename IncidenceGraphType::vertex_iterator, typename IncidenceGraphType::vertex_iterator>
	vertices(const IncidenceGraphType& g) {
		return vertices(g.innerGraph);
	}

	IncidenceGraphTemplate
	inline std::pair<typename IncidenceGraphType::adjacency_iterator, typename IncidenceGraphType::adjacency_iterator>
	adjacent_vertices(typename IncidenceGraphType::vertex_descriptor u, const IncidenceGraphType& g) {
		return adjacent_vertices(u, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline std::pair<typename IncidenceGraphType::adjacency_iterator, typename IncidenceGraphType::adjacency_iterator>
	in_adjacent_vertices(typename IncidenceGraphType::vertex_descriptor u, const IncidenceGraphType& g) {
		return adjacent_vertices(u, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename IncidenceGraphType::vertex_descriptor source(
		typename IncidenceGraphType::edge_descriptor e, const IncidenceGraphType& g) {
		return source(e, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename IncidenceGraphType::vertex_descriptor target(
		typename IncidenceGraphType::edge_descriptor e, const IncidenceGraphType& g) {
		return target(e, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline std::pair<typename IncidenceGraphType::out_edge_iterator, typename IncidenceGraphType::out_edge_iterator>
	out_edges(typename IncidenceGraphType::vertex_descriptor u, const IncidenceGraphType& g) {
		return out_edges(u, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline std::pair<typename IncidenceGraphType::in_edge_iterator, typename IncidenceGraphType::in_edge_iterator>
	in_edges(typename IncidenceGraphType::vertex_descriptor v, const IncidenceGraphType& g) {
		return out_edges(v, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename IncidenceGraphType::degree_size_type out_degree(
		typename IncidenceGraphType::vertex_descriptor u, const IncidenceGraphType& g) {
		return out_degree(u, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename IncidenceGraphType::degree_size_type in_degree(
		typename IncidenceGraphType::vertex_descriptor v, const IncidenceGraphType& g) {
		return out_degree(v, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename IncidenceGraphType::degree_size_type degree(
		typename IncidenceGraphType::vertex_descriptor v, const IncidenceGraphType& g) {
		return out_degree(v, g.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename IncidenceGraphType::vertices_size_type num_vertices(const IncidenceGraphType& g) {
		return num_vertices(g.innerGraph);
	}

	IncidenceGraphTemplate
	inline typename IncidenceGraphType::edges_size_type num_edges(const IncidenceGraphType& g) {
		return num_edges(g.innerGraph);
	}

	IncidenceGraphTemplate
	inline std::pair<typename IncidenceGraphType::edge_descriptor, bool> edge(
		typename IncidenceGraphType::vertex_descriptor u,
		typename IncidenceGraphType::vertex_descriptor v,
		const IncidenceGraphType& graph) {
		return edge(u, v, graph.innerGraph);
	}

	IncidenceGraphTemplate
	inline decltype(auto) add_edge(
		typename IncidenceGraphType::vertex_descriptor u,
		typename IncidenceGraphType::vertex_descriptor v,
		const IncidenceGraphType& graph) {
		return add_edge(u, v, graph.innerGraph);
	}

	IncidenceGraphTemplate
	inline decltype(auto) remove_edge(
		typename IncidenceGraphType::edge_descriptor e,
		const IncidenceGraphType& graph) {
		return remove_edge(e, graph.innerGraph);
	}

	template<typename Graph, typename VertexProperties, typename EdgeProperties, typename Predicate>
	inline decltype(auto) remove_out_edge_if(
		typename IncidenceGraphType::vertex_descriptor in_v,
		Predicate& predicate,
		const IncidenceGraphType& graph) {
		return remove_out_edge_if(in_v, predicate, graph.innerGraph);
	}

#undef IncidenceGraphType
#undef IncidenceGraphTemplate
}

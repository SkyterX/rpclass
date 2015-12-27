#pragma once
#include <graph/properties.hpp>
#include <graph/dijkstra.hpp>
#include <graph/dynamic_graph.hpp>

namespace ch
{
	template <typename Graph>
	using DefaultCHVisitor = graph::DefaultDijkstraVisitor<Graph>;

	enum class DirectionBit :char {
		both = 0,
		forward = 1,
		backward = 2,
	};

	template <typename PredecessorFMapTag, typename PredecessorBMapTag, typename DisanceMapFTag, class DisanceMapBTag,
	          typename WeightMapTag, typename IndexMapTag, typename ColorFMapTag, typename ColorBMapTag, typename UnPackMapTag,
	          typename VertexOrderMapTag, typename DirectionMapTag,
	          typename VP, typename EP, typename GP>
	struct GenerateCHGraph {};

	template <typename PredecessorFMapTag, typename PredecessorBMapTag, typename DisanceMapFTag, typename DisanceMapBTag,
	          typename WeightMapTag, typename IndexMapTag, typename ColorFMapTag, typename ColorBMapTag, typename UnPackMapTag,
	          typename VertexOrderMapTag, typename DirectionMapTag,
	          typename... P1s, typename... P2s, typename ... P3s>
	struct GenerateCHGraph<PredecessorFMapTag, PredecessorBMapTag, DisanceMapFTag, DisanceMapBTag, WeightMapTag,
	                       IndexMapTag, ColorFMapTag, ColorBMapTag, UnPackMapTag, VertexOrderMapTag, DirectionMapTag,
	                       graph::Properties<P1s...>, graph::Properties<P2s...>, graph::Properties<P3s...>> {
		using EmptyGraph = graph::BDynamicGraph<
			graph::Properties<>, graph::Properties<>, graph::Properties<>>;

		using vertex_descriptor =
		typename graph::graph_traits<EmptyGraph>::vertex_descriptor;
		using vertices_size_type =
		typename graph::graph_traits<EmptyGraph>::vertices_size_type;

		using type = graph::BDynamicGraph<
			graph::Properties<
				graph::Property<PredecessorFMapTag, vertex_descriptor>,
				graph::Property<PredecessorBMapTag, vertex_descriptor>,
				graph::Property<DisanceMapFTag, uint32_t>,
				graph::Property<DisanceMapBTag, uint32_t>,
				graph::Property<ColorFMapTag, boost::two_bit_color_type>,
				graph::Property<ColorBMapTag, boost::two_bit_color_type>,
				graph::Property<VertexOrderMapTag, vertices_size_type>,
				P1s...>,
			graph::Properties<
				graph::Property<WeightMapTag, uint32_t>,
				graph::Property<UnPackMapTag, vertex_descriptor>,
				graph::Property<DirectionMapTag, DirectionBit>,
				P2s...>,
			graph::Properties<P3s...>
		>;
	};
}

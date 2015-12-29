#pragma once

#include <map>
#include <graph/detail/IncidenceGraph.hpp>
#include <ch/ContractionHierarchyStructures.hpp>
#include <graph/graph.hpp>

namespace ch
{

	template <typename Graph, typename OrderMap, typename DirectionMap>
	struct CHPreprocessDijkstraVisitor : public graph::DefaultDijkstraVisitor<Graph> {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;

		CHPreprocessDijkstraVisitor(Vertex& ignoredVertex, size_t hopCount, OrderMap& order, DirectionMap& direction, DirectionBit ignoredDirection)
			: ignoredVertex(ignoredVertex), order(order), direction(direction), maxHopCount(hopCount), ignoredDirection(ignoredDirection) {
			currentHopCount = 0;
		}

		bool should_relax(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
			if (get(direction, edge) != ignoredDirection) {
				auto to = target(edge, graph);
				return get(order, to) > get(order, ignoredVertex);
			}
			return false;
		}

		bool should_continue() {
			return currentHopCount++ < maxHopCount;
		};

	private:
		Vertex ignoredVertex;
		OrderMap order;
		DirectionMap direction;
		DirectionBit ignoredDirection;
		size_t maxHopCount;
		size_t currentHopCount;
	};


	template <typename Graph, typename IndexMap,
		typename DijkstraVisitorF, typename DijkstraVisitorB,
		typename DistanceMapF, typename DistanceMapB,
		typename ColorMapF, typename ColorMapB>
	class CHPreprocessOptimalCriteriaTraker : public graph::OptimalCriteriaTraker<
		Graph, IndexMap,
		DijkstraVisitorF, DijkstraVisitorB,
		DistanceMapF, DistanceMapB,
		ColorMapF, ColorMapB> {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using DistanceType = typename DistanceMapF::value_type;
	public:

		CHPreprocessOptimalCriteriaTraker(
			const DijkstraVisitorF& visitorF, const DijkstraVisitorB& visitorB,
			DistanceMapF& distanceF, DistanceMapB& distanceB,
			ColorMapF& colorF, ColorMapB& colorB, const IndexMap& index)
			: graph::OptimalCriteriaTraker<
			Graph, IndexMap,
			DijkstraVisitorF, DijkstraVisitorB,
			DistanceMapF, DistanceMapB,
			ColorMapF, ColorMapB>(
				visitorF, visitorB,
				distanceF, distanceB,
				colorF, colorB, index) {}

		void edge_relaxed(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
			Vertex to = target(edge, graph);
			if (!this->visitorF.Stored.VertexInitializer.IsInitialized(to, this->index) ||
				!this->visitorB.Stored.VertexInitializer.IsInitialized(to, this->index) ||
				get(this->colorF, to) == boost::two_bit_white ||
				get(this->colorB, to) == boost::two_bit_white)
				return;
			uint32_t right = get(this->distanceF, to) + get(this->distanceB, to);
			if (right < this->mu) {
				this->mu = right;
				this->transitNode = to;
			}
		};
	};

	template <typename Graph>
	class EdgeHash {
		using EdgeDescriptor = typename graph::graph_traits<Graph>::edge_descriptor;
		using VertexDescriptor = typename graph::graph_traits<Graph>::vertex_descriptor;
		using VertexPair = std::pair<VertexDescriptor, VertexDescriptor>;
		Graph* g;
		std::hash<VertexPair> hasher;
	public:

		explicit EdgeHash(Graph* g) : g(g) {}

		inline size_t operator()(const EdgeDescriptor& e) const {
			return hasher(make_pair(source(e, *g), target(e, *g)));

		}
	};
};
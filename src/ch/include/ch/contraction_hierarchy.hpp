#pragma once

#include <graph/dijkstra.hpp>
#include <boost/graph/two_bit_color_map.hpp>
#include <graph/bidirectional_dijkstra.hpp>
#include <graph/detail/IncidenceGraph.hpp>
#include <ch/ContractionHierarchyStructures.hpp>
#include <ch/ContractionHierarchyOrderings.hpp>
#include <ch/ContractionHierarchyPreprocessing.hpp>

namespace ch {



template <typename Graph, typename OrderMap, typename DirectionMap>
struct CHQueryVisitor : public graph::DefaultDijkstraVisitor<Graph> {
	using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;

	CHQueryVisitor(OrderMap& order, DirectionMap& direction, DirectionBit ignoredDirection)
		: ignoredDirection(ignoredDirection), order(order), direction(direction) {}

	bool should_relax(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
		auto to = target(edge, graph);
		auto from = source(edge, graph);
//		std::cout << "\tShould " << (ignoredDirection == DirectionBit::backward ? "forward" : "backward") << " search relax edge "
//			<< from + 1 << " " << to + 1 << " "
//			<< (get(direction, edge) == DirectionBit::backward ? "backward" : "forward") << std::endl;
		if (get(direction, edge) != ignoredDirection) {
			if (ignoredDirection != DirectionBit::backward && get(order, to) > get(order, from)) {
//				std::cout << "\tRelaxing edge backward: " << to + 1 << " " << from + 1 << std::endl;
				return true;
			}
			if (ignoredDirection != DirectionBit::forward && get(order, to) > get(order, from)) {
//				std::cout << "\tRelaxing edge forward: " << from + 1 << " " << to + 1 << std::endl;
				return true;
			}
		}
		return false;
	}

private:
	DirectionBit ignoredDirection;
	OrderMap order;
	DirectionMap direction;
};

	template <typename Graph, typename IndexMap,
	          typename DijkstraVisitorF, typename DijkstraVisitorB,
	          typename DistanceMapF, typename DistanceMapB,
	          typename ColorMapF, typename ColorMapB>
	class CHOptimalCriteriaTraker : public graph::OptimalCriteriaTraker<
				Graph, IndexMap,
				DijkstraVisitorF, DijkstraVisitorB,
				DistanceMapF, DistanceMapB,
				ColorMapF, ColorMapB> {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using DistanceType = typename DistanceMapF::value_type;
	public:

		CHOptimalCriteriaTraker(
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

		bool should_continue() {
			if (this->visitorF.Stored.Queue.IsEmpty() || this->visitorB.Stored.Queue.IsEmpty())
				return true;
			auto& topItem = this->direction_flag_forward
				? this->visitorF.Stored.Queue.PeekMin()
				: this->visitorB.Stored.Queue.PeekMin();
			return !(this->mu <= topItem.Distance);
		}
	};


template <typename Graph, typename DirectionMap, typename WeightMap>
struct IncreaseWeightOfIncommingEdgeVisitor:public graph::DefaultDijkstraVisitor<Graph> {
    IncreaseWeightOfIncommingEdgeVisitor(DirectionMap& direction, WeightMap weight)
        :direction(direction),weight(weight) {};
void examine_edge(const typename graph::graph_traits<Graph>::edge_descriptor& e, const Graph& graph) {
        if (get(direction, e) == DirectionBit::backward)
            graph::put(weight, e, 10000000);
    }
    DirectionMap direction;
    WeightMap weight;
};

template <typename Graph, typename PredecessorFMap, typename PredecessorBMap, typename DistanceFMap,
    typename DistanceBMap, typename WeightMap, typename IndexMap, typename ColorFMap, typename ColorBMap, typename UnPackMap,
    typename VertexOrderMap, typename DirectionMap, typename CHVisitor = DefaultCHVisitor<Graph>>
    void ch_query(Graph& originalGraph, 
        const typename graph::graph_traits<Graph>::vertex_descriptor& s,
        const typename graph::graph_traits<Graph>::vertex_descriptor& t,
        PredecessorFMap& predecessorF, PredecessorBMap& predecessorB, DistanceFMap& distanceF, DistanceBMap& distanceB,
        WeightMap& weight, IndexMap& index, ColorFMap& colorF, ColorBMap& colorB, UnPackMap& unpack,
        VertexOrderMap& order, DirectionMap& direction, CHVisitor&& visitor = CHVisitor()) {
        //IncreaseWeightOfIncommingEdgeVisitor<Graph,DirectionMap,WeightMap> dijVisitor(direction, weight);
        //graph::dijkstra(graph, s, predecessor, distanceF, weight, index, color, dijVisitor);
	using VisitorFType = CHQueryVisitor<Graph, VertexOrderMap, DirectionMap>;
	using VisitorBType = CHQueryVisitor<Graph, VertexOrderMap, DirectionMap>;
	using TrackerType = CHOptimalCriteriaTraker<Graph, IndexMap, VisitorFType, VisitorBType, DistanceFMap, DistanceBMap, ColorFMap, ColorBMap>;
	auto graph = graph::IncidenceGraph<Graph>(originalGraph);

	auto chFVisitor = VisitorFType(order, direction, DirectionBit::backward);
	auto chBVisitor = VisitorBType(order, direction, DirectionBit::forward);

	graph::fancy_bidirectional_dijkstra<TrackerType>(graph, graph, s, t, predecessorF, predecessorB, distanceF, distanceB, weight, index, colorF, colorB, chFVisitor, chBVisitor);
//	graph::dijkstra(graph, s, predecessorF, distanceF, weight, index, colorF, chFVisitor);

    };
};
#pragma once
#include <map>
#include <graph/detail/IncidenceGraph.hpp>
#include <graph/detail/util/Collection.hpp>
#include <ch/ContractionHierarchyOrderings.hpp>
#include <ch/ContractionHierarchyStructures.hpp>

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


	// uses dijkstra, therefore should have at least all property maps used by dijkstra
	template <typename Graph, typename PredecessorFMap, typename PredecessorBMap, typename DistanceFMap,
	          typename DistanceBMap, typename WeightMap, typename IndexMap, typename ColorFMap, typename ColorBMap, typename UnPackMap,
	          typename VertexOrderMap, typename DirectionMap,
	          typename OrderStrategy = ShortCutOrderStrategy<Graph>>
	void ch_preprocess(Graph& originalGraph,
	                   PredecessorFMap& predecessorF, PredecessorBMap& predecessorB,
	                   DistanceFMap& distanceF, DistanceBMap& distanceB,
	                   WeightMap& weight, IndexMap& index,
	                   ColorFMap& colorF, ColorBMap& colorB,
	                   UnPackMap& unpack, VertexOrderMap& order,
	                   DirectionMap& direction, size_t dijLimit,
	                   OrderStrategy&& strategy = OrderStrategy()) {
		using namespace std;
		using namespace graphUtil;
		using namespace graph;

		using Vertex = typename graph_traits<Graph>::vertex_descriptor;
		using DijkstraVisitorType = CHPreprocessDijkstraVisitor<Graph, VertexOrderMap, DirectionMap>;
		using TrackerType = CHPreprocessOptimalCriteriaTraker<Graph, IndexMap, DijkstraVisitorType, DijkstraVisitorType, DistanceFMap, DistanceBMap, ColorFMap, ColorBMap>;

		auto graph = CreateIncidenceGraph(originalGraph);
		auto curVert = strategy.next(graph);

		for (const auto& vertex : Range(vertices(graph))) {
			graph::put(order, vertex, numeric_limits<size_t>::max());
		}

		size_t curOrder = 0;
		int counter = 0;
		while (curVert != graph.null_vertex()) {
			graph::put(order, curVert, curOrder++);
			++counter;
			if (counter % 1000 == 0)
				cout << "Processing " << counter << " vertex Id : " << curVert + 1 << " Degree : " << out_degree(curVert, graph) << endl;

			map<pair<Vertex, Vertex>, size_t> shortCuts;
			for (const auto& out_e : Range(out_edges(curVert, graph))) {
				if (get(direction, out_e) == DirectionBit::backward) {
					continue;
				}
				for (const auto& in_e : Range(out_edges(curVert, graph))) {
					if (get(direction, in_e) == DirectionBit::forward) {
						continue;
					}
					if (in_e == out_e) {
						continue;
					}

					auto in_v = target(in_e, graph);
					auto out_v = target(out_e, graph);
					auto curVertexOrder = get(order, curVert);
					if (in_v == out_v
						|| get(order, in_v) <= curVertexOrder
						|| get(order, out_v) <= curVertexOrder)
						continue;

					auto shortCutLength = get(weight, out_e) + get(weight, in_e);

					DijkstraVisitorType visitorF(curVert, dijLimit, order, direction, DirectionBit::backward);
					DijkstraVisitorType visitorB(curVert, dijLimit, order, direction, DirectionBit::forward);

					graph::fancy_bidirectional_dijkstra<TrackerType>(
						graph, graph, in_v, out_v, 
						predecessorF, predecessorB, distanceF, distanceB, 
						weight, index, colorF, colorB, visitorF, visitorB);

					if (get(colorF, out_v) == boost::two_bit_black) {
						auto dist = get(distanceF, out_v);
						if (dist < shortCutLength) {
//						cout << "\tShortcut NOT added: " << in_v + 1 << " to " << out_v + 1 << " with length " << shortCutLength << endl;
							continue;
						}
					}

//				cout << "\tShortcut added: " << in_v + 1 << " to " << out_v + 1 << " with length " << shortCutLength << endl;
					auto shortCutKey = make_pair(in_v, out_v);
					auto it = shortCuts.find(shortCutKey);
					if (it == shortCuts.end() || it->second > shortCutLength)
						shortCuts[shortCutKey] = shortCutLength;

				}
			}
			//remove edges
			for (const auto& edge : AsArray(Range(out_edges(curVert, graph)))) {
				auto to = target(edge, graph);
				auto from = source(edge, graph);
				if (get(order, to) <= get(order, from)) {
					graph::remove_edge(edge, graph);
				}
			}

			for (auto& shortCut : shortCuts) {
				auto u = shortCut.first.first;
				auto v = shortCut.first.second;
				auto shortCutWeight = shortCut.second;
//			cout << "\tShortcut added: " << u + 1 << " to " << v + 1 << " with length " << shortCutWeight << endl;
				auto pr = graph::add_edge(u, v, graph);
				graph::put(weight, pr.first, shortCutWeight);
				graph::put(direction, pr.first, DirectionBit::forward);
				graph::put(unpack, pr.first, curVert);

				auto pr1 = graph::add_edge(v, u, graph);
				graph::put(weight, pr1.first, shortCutWeight);
				graph::put(direction, pr1.first, DirectionBit::backward);
				graph::put(unpack, pr1.first, curVert);
			}
//		cin.get();

			curVert = strategy.next(graph);
		}

	};
}

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

		CHPreprocessDijkstraVisitor(Vertex& ignoredVertex, Vertex& finishVertex, size_t hopCount, OrderMap& order, DirectionMap& direction)
			: ignoredVertex(ignoredVertex), finishVertex(finishVertex), order(order), direction(direction), maxHopCount(hopCount) {
			currentHopCount = 0;
		}

		bool should_relax(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
			auto to = target(edge, graph);

			if (get(direction, edge) == DirectionBit::backward) {
				return false;
			}

			return get(order, to) > get(order, ignoredVertex);
		}

		void examine_vertex(const Vertex& examinedV, const Graph&) {
			lastRelaxedV = examinedV;
		};

		bool should_continue() {
			if (lastRelaxedV == finishVertex)
				return false;

			return currentHopCount++ < maxHopCount;

		};


	private:
		Vertex ignoredVertex;
		Vertex finishVertex;
		Vertex lastRelaxedV;
		OrderMap order;
		DirectionMap direction;
		size_t maxHopCount;
		size_t currentHopCount;
	};


	// uses dijkstra, therefore should have at least all property maps used by dijkstra
	template <typename Graph, typename PredecessorMap, typename DistanceMap,
	          typename WeightMap, typename IndexMap, typename ColorMap, typename UnPackMap,
	          typename VertexOrderMap, typename DirectionMap,
	          typename OrderStrategy = ShortCutOrderStrategy<Graph>>
	void ch_preprocess(Graph& originalGraph, PredecessorMap& predecessor, DistanceMap& distance,
	                   WeightMap& weight, IndexMap& index, ColorMap& color, UnPackMap& unpack,
	                   VertexOrderMap& order, DirectionMap& direction, size_t dijLimit,
	                   OrderStrategy&& strategy = OrderStrategy()) {
		using namespace std;
		using namespace graphUtil;
		using namespace graph;

		using Vertex = typename graph_traits<Graph>::vertex_descriptor;

		auto graph = IncidenceGraph<Graph>(originalGraph);
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
					if (get(order, in_v) <= curVertexOrder
						|| get(order, out_v) <= curVertexOrder)
						continue;

					auto shortCutLength = get(weight, out_e) + get(weight, in_e);

					CHPreprocessDijkstraVisitor<Graph, VertexOrderMap, DirectionMap> visitor(curVert, out_v, dijLimit, order, direction);
					graph::dijkstra(graph, in_v, predecessor, distance, weight, index, color, visitor);

					// One can't just simply use dijkstra
					EnsureVertexInitialization(graph, out_v, predecessor, distance, index, color, visitor);
					if (get(color, out_v) == boost::two_bit_black) {
						auto dist = get(distance, out_v);
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

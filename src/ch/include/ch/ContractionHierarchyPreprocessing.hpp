#pragma once
#include <map>
#include <graph/detail/IncidenceGraph.hpp>
#include <graph/detail/util/Collection.hpp>
#include <ch/ch_util.hpp>
#include <ch/ContractionHierarchyOrderings.hpp>
#include <ch/ContractionHierarchyStructures.hpp>

namespace ch
{
	template <typename Graph, typename DirectionMap, typename WeightMap>
	void RemovePairedEdges(Graph& graph, DirectionMap& direction, WeightMap& weight) {
		using Edge = typename graph::graph_traits<Graph>::edge_descriptor;
		for (const auto& curVert : graphUtil::Range(vertices(graph))) {
			for (const Edge& edge : AsArray(graphUtil::Range(out_edges(curVert, graph)))) {
				auto out_vertex = target(edge, graph);

				if (get(direction, edge) != DirectionBit::backward) {
					graph::remove_out_edge_if(curVert, [&out_vertex, &graph, &edge, &direction, &weight](const Edge& other_edge)-> bool {
						auto other_vertex = target(other_edge, graph);
						return other_vertex == out_vertex 
							&& get(direction, other_edge) != DirectionBit::backward
							&& edge != other_edge
							&& get(weight, other_edge) >= get(weight, edge);
					}, graph);
				}

				if (get(direction, edge) != DirectionBit::forward) {
					graph::remove_out_edge_if(curVert, [&out_vertex, &graph, &edge, &direction, &weight](const Edge& other_edge)-> bool {
						auto other_vertex = target(other_edge, graph);
						return other_vertex == out_vertex
							&& get(direction, other_edge) != DirectionBit::forward 
							&& edge != other_edge
							&& get(weight, other_edge) >= get(weight, edge);
					}, graph);
				}
			}
		}
	}

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
		RemovePairedEdges(graph, direction, weight);
		auto curVert = strategy.next(graph);
		
		for (const auto& vertex : Range(vertices(graph))) {
			graph::put(order, vertex, numeric_limits<typename VertexOrderMap::value_type>::max());
		}
		
//		for (auto v : Range(vertices(graph))) {
//			DumpEdges(v, graph, weight, direction);
//		}

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
					if (it == shortCuts.end() || it->second > shortCutLength) {
						shortCuts[shortCutKey] = shortCutLength;
						graph::remove_out_edge_if(in_v, [&graph, &out_v, &direction](const auto& edge)-> bool {
							auto out_vertex = target(edge, graph);
							return out_v == out_vertex && get(direction, edge) != DirectionBit::backward;
						}, graph);
						graph::remove_out_edge_if(out_v, [&graph, &in_v, &direction](const auto& edge)-> bool {
							auto in_vertex = target(edge, graph);
							return in_v == in_vertex && get(direction, edge) != DirectionBit::forward;
						}, graph);
					}
				}
			}
			//remove edges
//			cout << "Original Edges" << endl;
//			DumpEdges(curVert, graph, weight, direction);
			for (const auto& edge : AsArray(Range(out_edges(curVert, graph)))) {
				auto to = target(edge, graph);
				auto from = source(edge, graph);
				if (get(order, to) <= get(order, from)) {
					graph::remove_edge(edge, graph);
				}
			}
//			cout << "After deletion" << endl;
//			DumpEdges(curVert, graph, weight, direction);

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

//		for(auto v : Range(vertices(graph))) {
//			DumpEdges(v, graph, weight, direction);
//		}

		optimize_space(graph);

	};

	template<typename Graph, typename WeightMap, typename DirectionMap>
	void DumpEdges(const typename Graph::vertex_descriptor& v, Graph& g, WeightMap& weight, DirectionMap& direction) {
		using namespace std;
		using namespace graphUtil;
		cout << v << " : ";
		for (const auto &e : Range(out_edges(v, g))) {
			cout << "\t[" <<
				target(e, g) << ", " <<
				get(weight, e) << ", " <<
				(get(direction, e) == DirectionBit::forward ? "f" : "b") <<
				"], " << endl;
		}
		cout << endl;
	}
}

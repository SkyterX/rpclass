#pragma once
#include <map>
#include <graph/detail/IncidenceGraph.hpp>
#include <graph/detail/util/Collection.hpp>
#include <ch/ContractionHierarchyUtils.hpp>
#include <ch/ContractionHierarchyOrderings.hpp>
#include <ch/ContractionHierarchyStructures.hpp>

namespace ch
{
	template <typename Graph, typename WeightMap, typename DirectionMap>
	void DumpEdges(const typename Graph::vertex_descriptor& v, Graph& g, WeightMap& weight, DirectionMap& direction) {
		using namespace std;
		using namespace graphUtil;
		cout << v << " : ";
		for (const auto& e : Range(out_edges(v, g))) {
			cout << "\t[" <<
				target(e, g) << ", " <<
				get(weight, e) << ", " <<
				(get(direction, e) == DirectionBit::forward ? "f" : "b") <<
				"], " << endl;
		}
		cout << endl;
	}

	template <typename Graph, typename DirectionMap, typename WeightMap>
	void RemoveParallelEdges(Graph& graph, DirectionMap& direction, WeightMap& weight) {
		using namespace graphUtil;
		using namespace graph;
		using Edge = typename graph_traits<Graph>::edge_descriptor;
		for (const auto& curVert : Range(vertices(graph))) {
			for (const Edge& e : AsArray(Range(out_edges(curVert, graph)))) {
				auto out_vertex = target(e, graph);
				remove_out_edge_if(curVert, [&out_vertex, &graph, &e, &direction, &weight](const Edge& other_edge)-> bool {
					                   auto other_vertex = target(other_edge, graph);
					                   auto edgeDirection = get(direction, other_edge);
					                   return other_vertex == out_vertex
							                   && edgeDirection == get(direction, e)
							                   && e != other_edge
							                   && get(weight, other_edge) >= get(weight, e);
				                   }, graph);
			}
		}
	}

	template <typename Graph, typename DirectionMap, typename WeightType, typename Vertex, typename WeightMap, typename UnpackMap>
	void AddShortCut(Graph& graph, DirectionMap& direction, WeightMap& weight, UnpackMap& unpack,
	                 Vertex& sourceVertex, Vertex& targetVertex, WeightType& shortCutWeight, Vertex& viaVertex,
	                 DirectionBit shortcutDirection) {
		// if i am adding shortcut, previous edges are larger -> remove them
		graph::remove_out_edge_if(sourceVertex,
		                          [&graph, &targetVertex, &direction, &shortcutDirection]
		                          (const auto& e)-> bool {
			                          auto out_vertex = target(e, graph);
			                          return targetVertex == out_vertex && get(direction, e) == shortcutDirection;
		                          }, graph);

		auto pr = graph::add_edge(sourceVertex, targetVertex, graph);
		graph::put(weight, pr.first, shortCutWeight);
		graph::put(direction, pr.first, shortcutDirection);
		graph::put(unpack, pr.first, viaVertex);

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
		using WeightType = typename WeightMap::value_type;
		using VertexOrderType = typename VertexOrderMap::value_type;

		auto graph = CreateIncidenceGraph(originalGraph);
		// removeBothTypeEdges
		RemoveParallelEdges(graph, direction, weight);

		for (const auto& vertex : Range(vertices(graph))) {
			graph::put(order, vertex, numeric_limits<VertexOrderType>::max());
		}

		//		for (auto v : Range(vertices(graph))) {
		//			DumpEdges(v, graph, weight, direction);
		//		}

		VertexOrderType curOrder = 0;
		int counter = 0;
		int shortcutAmount = 0;
		auto curVert = strategy.next(graph);
		while (curVert != graph.null_vertex()) {
			graph::put(order, curVert, curOrder++);
			++counter;
			if (counter % 1000 == 0) {
				cout << "Processing " << counter << " vertex Id : " << curVert + 1 << " Degree : " << out_degree(curVert, graph) << endl;
				cout << "Shortcuts amount " << shortcutAmount << endl;
			}

			// find possible shortcuts
			map<pair<Vertex, Vertex>, WeightType> shortcutCandidates;
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
					auto shortcutKey = std::make_pair(in_v, out_v);
					auto it = shortcutCandidates.find(shortcutKey);
					if (it == shortcutCandidates.end() || it->second > shortCutLength)
						shortcutCandidates[shortcutKey] = shortCutLength;
				}
			}

			// adding new shortcuts // no problems with descriptors
			for (const auto& it : shortcutCandidates) {
				auto in_v = it.first.first;
				auto out_v = it.first.second;
				auto shortCutLength = it.second;

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
				shortcutAmount++;
				//					cout << "\tShortcut added: " << in_v + 1 << " to " << out_v + 1 << " with length " << shortCutLength << endl;
				AddShortCut(graph, direction, weight, unpack, in_v, out_v, shortCutLength, curVert, DirectionBit::forward);
				AddShortCut(graph, direction, weight, unpack, out_v, in_v, shortCutLength, curVert, DirectionBit::backward);

			}


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
			curVert = strategy.next(graph);
		}

		//		for(auto v : Range(vertices(graph))) {
		//			DumpEdges(v, graph, weight, direction);
		//		}

		optimize_space(graph);

		cout << "Shortcuts amount " << shortcutAmount << endl;
	};

}


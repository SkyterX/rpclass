#pragma once
#include <graph/dijkstra.hpp>
#include <graph/detail/ComplementGraph.hpp>

namespace graph
{
	template <typename PredecessorMapFTag, typename PredecessorMapBTag,
	          typename DisanceMapFTag, typename DisanceMapBTag, typename WeightMapTag,
	          typename IndexMapTag, typename ColorMapFTag, typename ColorMapBTag,
	          typename BundledVertexProperties, typename BundledEdgeProperties>
	struct GenerateBiDijkstraGraph {};


	template <typename PredecessorMapFTag, typename PredecessorMapBTag,
	          typename DisanceMapFTag, typename DisanceMapBTag,
	          typename WeightMapTag, typename IndexMapTag, typename ColorMapFTag,
	          typename ColorMapBTag, typename... P1s, typename... P2s>
	struct GenerateBiDijkstraGraph<PredecessorMapFTag, PredecessorMapBTag,
	                               DisanceMapFTag, DisanceMapBTag, WeightMapTag,
	                               IndexMapTag, ColorMapFTag, ColorMapBTag, Properties<P1s...>, Properties<P2s...>> {
		using type = StaticGraph<
			Properties<
				Property<PredecessorMapFTag,
				         typename graph_traits<StaticGraph<Properties<>, Properties<>>>::vertex_descriptor>,
				Property<PredecessorMapBTag,
				         typename graph_traits<StaticGraph<Properties<>, Properties<>>>::vertex_descriptor>,
				Property<DisanceMapFTag, uint32_t>,
				Property<DisanceMapBTag, uint32_t>,
				Property<ColorMapFTag, boost::two_bit_color_type>,
				Property<ColorMapBTag, boost::two_bit_color_type>,
				P1s...>,
			Properties<
				Property<WeightMapTag, uint32_t>,
				P2s...>>;
	};

	template <typename Graph,
	          class DistanceMapF, class DistanceMapB>
	class OptimalCriteriaTraker {
		using Vertex = typename graph_traits<Graph>::vertex_descriptor;
	public :
		void edge_relaxed(const typename graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
			Vertex to = target(edge, graph);
			uint32_t right = get(distanceF, to) + get(distanceB, to);
			if (right < mu) {
				mu = right;
				m_center = to;
			}
		};

		bool should_continue() {
			Vertex v1, v2;
			int din, dout;
			std::tie(din, v1) = queueF.PeekMin();
			std::tie(dout, v2) = queueB.PeekMin();
			return !(mu <= din + dout);
		};

		//can be easily changed to min queue iterator or something like it
		bool forward_iteration_is_next() {
			direction_flag_forward = !direction_flag_forward;
			return direction_flag_forward;
		}

		OptimalCriteriaTraker(const queue::HeapQueue<int, Vertex>& queue_f, const queue::HeapQueue<int, Vertex>& queue_b,
		                      DistanceMapF& distance_f, DistanceMapB& distance_b)
			: queueF(queue_f),
			  queueB(queue_b),
			  distanceF(distance_f),
			  distanceB(distance_b) {
			mu = std::numeric_limits<uint32_t>::max() >> 2;
			direction_flag_forward = false;
		}

		uint32_t mu;
		Vertex m_center;
		bool direction_flag_forward;
		queue::HeapQueue<int, Vertex> queueF;
		queue::HeapQueue<int, Vertex> queueB;
		DistanceMapF& distanceF;
		DistanceMapB& distanceB;
	};


	template <class Graph, class PredecessorMap, class DistanceMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor>
	void init_one_vertex(Graph& graph,
	                     const typename graph_traits<Graph>::vertex_descriptor& v,
	                     PredecessorMap& predecessor,
	                     DistanceMap& distance,
	                     IndexMap& index, ColorMap& color,
	                     DijkstraVisitor visitor
	) {
		visitor.initialize_vertex(v, graph);
		put(distance, v, std::numeric_limits<typename graph_traits<Graph>::vertex_descriptor>::max() >> 2);
		put(predecessor, v, v);
		put(color, v, boost::two_bit_color_type::two_bit_white);
	}

	template <class Graph, class DistanceMap, class IndexMap,
	          class ColorMap, class DijkstraVisitor, class Queue>
	void init_first_vertex(Graph& graph,
	                       const typename graph_traits<Graph>::vertex_descriptor& v,
	                       DistanceMap& distance,
	                       IndexMap& index, ColorMap& color,
	                       DijkstraVisitor visitor, Queue& queue) {
		visitor.discover_vertex(v, graph);
		put(distance, v, 0);
		put(color, v, boost::two_bit_color_type::two_bit_green);
		queue.Insert(0, v);
	}


	template <class Graph, class PredecessorMap, class DistanceMap, class WeightMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor, typename BidijkstraVisitor>
	bool dijkstra_iteration(Graph& graph,
	                        PredecessorMap& predecessor, DistanceMap& distance, WeightMap& weight,
	                        IndexMap& index, ColorMap& color, DijkstraVisitor visitor,
	                        queue::HeapQueue<int, typename graph_traits<Graph>::vertex_descriptor>& queue,
	                        BidijkstraVisitor& bivisitor) {
		using Vertex = typename graph_traits<Graph>::vertex_descriptor;

		// Get vertex from queue
		Vertex v;
		int vDistance;
		std::tie(vDistance, v) = queue.PeekMin();
		queue.DeleteMin();
		visitor.examine_vertex(v, graph);

		// Process edges	
		for (const auto& edge : graphUtil::Range(out_edges(v, graph))) {
			visitor.examine_edge(edge, graph);
			if (!visitor.should_relax(edge, graph))
				continue;
			// Get edge Properties
			Vertex to = target(edge, graph);
			auto edgeWeight = get(weight, edge);
			auto newDistance = vDistance + edgeWeight;
			auto toDistance = get(distance, to);
			if (newDistance < toDistance) {
				// Found better distance -> update
				put(distance, to, newDistance);
				put(predecessor, to, v);
				if (get(color, to) == boost::two_bit_color_type::two_bit_white) {
					// Vertex is new
					visitor.discover_vertex(to, graph);
					put(color, to, boost::two_bit_color_type::two_bit_green);
					queue.Insert(newDistance, to);
				}
				else {
					queue.DecreaseKey(toDistance, to, newDistance);
				}
				visitor.edge_relaxed(edge, graph);
				bivisitor.edge_relaxed(edge, graph);
			}
			else {
				// Found same or worse distance
				visitor.edge_not_relaxed(edge, graph);
			}
		}

		// Teardown vertex
		put(color, v, boost::two_bit_color_type::two_bit_black);
		visitor.finish_vertex(v, graph);
		if (!visitor.should_continue())
			return false;

		return bivisitor.should_continue();
	};


	template <class Graph, class PredecessorMapF, class PredecessorMapB,
	          class DistanceMapF, class DistanceMapB,
	          class WeightMap, class IndexMap, class ColorMapF, class ColorMapB,
	          class DijkstraVisitorF = DefaultDijkstraVisitor<Graph>,
	          class DijkstraVisitorB = DefaultDijkstraVisitor<Graph>>
	void bidirectional_dijkstra(Graph& graph,
	                            const typename graph_traits<Graph>::vertex_descriptor& s,
	                            const typename graph_traits<Graph>::vertex_descriptor& t,
	                            PredecessorMapF& predecessorF, PredecessorMapB& predecessorB,
	                            DistanceMapF& distanceF, DistanceMapB& distanceB, WeightMap& weight,
	                            IndexMap& index, ColorMapF& colorF, ColorMapB& colorB,
	                            DijkstraVisitorF visitorF = DijkstraVisitorF(),
	                            DijkstraVisitorF visitorB = DijkstraVisitorB()) {

		if (s == t) {
			put(distanceF, t, 0);
			put(predecessorF, t, t);
			return;
		}
		using Vertex = typename graph_traits<Graph>::vertex_descriptor;

		queue::HeapQueue<int, Vertex> queueF(num_vertices(graph));
		queue::HeapQueue<int, Vertex> queueB(num_vertices(graph));

		for (auto& v : graphUtil::Range(vertices(graph))) {
			init_one_vertex(graph, v, predecessorF, distanceF, index, colorF, visitorF);
			init_one_vertex(graph, v, predecessorB, distanceB, index, colorB, visitorB);
		}

		// Process start vertex
		init_first_vertex(graph, s, distanceF, index, colorF, visitorF, queueF);
		init_first_vertex(graph, t, distanceB, index, colorB, visitorB, queueB); // should it be inverted graph?

		auto invertedGraph = graph::ComplementGraph<Graph>(graph);
		OptimalCriteriaTraker<Graph, DistanceMapF, DistanceMapB> optTracker(queueF, queueB, distanceF, distanceB);

		while (!queueF.IsEmpty() && !queueB.IsEmpty()) {
			if (optTracker.forward_iteration_is_next()) {
				bool res = dijkstra_iteration(graph, predecessorF, distanceF, weight, index, colorF, visitorF, queueF, optTracker);
				if (!res)
					break;
			}
			else {
				bool res = dijkstra_iteration(invertedGraph, predecessorB, distanceB, weight, index, colorB, visitorB, queueB, optTracker);
				if (!res)
					break;
			}
		}


		//emulate simple dijkstra
		uint32_t dis = optTracker.mu;
		put(distanceF, t, dis);


		Vertex predecessor = optTracker.m_center;
		Vertex current = get(predecessorB, predecessor);
		while (current != t) {
			put(predecessorF, current, predecessor);
			predecessor = current;
			current = get(predecessorB, current);
		}
	}
}

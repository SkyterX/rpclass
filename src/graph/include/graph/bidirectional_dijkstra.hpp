#pragma once
#include <graph/dijkstra.hpp>

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
			visitorF.initialize_vertex(v, graph);
			put(distanceF, v, std::numeric_limits<Vertex>::max()>>2);
			put(predecessorF, v, v);
			put(colorF, v, boost::two_bit_color_type::two_bit_white);
			visitorB.initialize_vertex(v, graph);
			put(distanceB, v, std::numeric_limits<Vertex>::max()>>2);
			put(predecessorB, v, v);
			put(colorB, v, boost::two_bit_color_type::two_bit_white);
		}

		// Process start vertex
		visitorF.discover_vertex(s, graph);
		put(distanceF, s, 0);
		put(colorF, s, boost::two_bit_color_type::two_bit_green);
		queueF.Insert(0, s);
		visitorB.discover_vertex(t, graph);
		put(distanceB, t, 0);
		put(colorB, t, boost::two_bit_color_type::two_bit_green);
		queueB.Insert(0, t);

		bool direction_flag_forward = false;
		uint32_t mu = std::numeric_limits<uint32_t>::max() >> 2; // distance type
		bool muIsOpt = false;
		Vertex m_center;
		//out min di[v]+do[v]
		//in min di[u]+do[u]
		// peek min queue sum <=

		while (!muIsOpt && !(queueF.IsEmpty() || queueB.IsEmpty())) {
			direction_flag_forward = !direction_flag_forward;
			if (direction_flag_forward) {
				// Get vertex from queue
				Vertex v;
				int vDistance;
				std::tie(vDistance, v) = queueF.PeekMin();
				queueF.DeleteMin();
				visitorF.examine_vertex(v, graph);

				// Process edges	
				for (const auto& edge : graphUtil::Range(out_edges(v, graph))) {
					visitorF.examine_edge(edge, graph);
					if (!visitorF.should_relax(edge, graph))
						continue;
					// Get edge Properties
					Vertex to = target(edge, graph);
					auto edgeWeight = get(weight, edge);
					auto newDistance = vDistance + edgeWeight;
					auto toDistance = get(distanceF, to);
					if (newDistance < toDistance) {
						// Found better distance -> update
						visitorF.edge_relaxed(edge, graph);
						put(distanceF, to, newDistance);
						put(predecessorF, to, v);

						uint32_t right = get(distanceF, to) + get(distanceB, to);
						if (right < mu) {
							mu = right;
							m_center = to;
						}

						if (get(colorF, to) == boost::two_bit_color_type::two_bit_white) {
							// Vertex is new
							visitorF.discover_vertex(to, graph);
							put(colorF, to, boost::two_bit_color_type::two_bit_green);
							queueF.Insert(newDistance, to);
						}
						else {
							queueF.DecreaseKey(toDistance, to, newDistance);
						}

					}
					else {
						// Found same or worse distance
						visitorF.edge_not_relaxed(edge, graph);
					}

				}

				// Teardown vertex
				put(colorF, v, boost::two_bit_color_type::two_bit_black);
				visitorF.finish_vertex(v, graph);
				if (!visitorF.should_continue())
					break;

			}
			else {
				// Get vertex from queue
				Vertex v;
				int vDistance;
				std::tie(vDistance, v) = queueB.PeekMin();
				queueB.DeleteMin();
				visitorB.examine_vertex(v, graph);

				// Process edges	
				for (const auto& edge : graphUtil::Range(in_edges(v, graph))) {
					visitorB.examine_edge(edge, graph);
					if (!visitorB.should_relax(edge, graph))
						continue;
					// Get edge Properties
					Vertex from = source(edge, graph);
					auto edgeWeight = get(weight, edge);
					auto newDistance = vDistance + edgeWeight;
					auto fromDistance = get(distanceB, from);
					if (newDistance < fromDistance) {
						// Found better distance -> update
						visitorB.edge_relaxed(edge, graph);
						put(distanceB, from, newDistance);
						put(predecessorB, from, v);
						if (get(colorB, from) == boost::two_bit_color_type::two_bit_white) {
							// Vertex is new
							visitorB.discover_vertex(from, graph);
							put(colorB, from, boost::two_bit_color_type::two_bit_green);
							queueB.Insert(newDistance, from);
						}
						else {
							queueB.DecreaseKey(fromDistance, from, newDistance);
						}
					}
					else {
						// Found same or worse distance
						visitorB.edge_not_relaxed(edge, graph);
					}
					uint32_t right = get(distanceF, from) + get(distanceB, from);
					if (right < mu) {
						mu = right;
						m_center = from;
					}
				}
				// Teardown vertex
				put(colorB, v, boost::two_bit_color_type::two_bit_black);
				visitorB.finish_vertex(v, graph);
				if (!visitorB.should_continue())
					break;


			}

			Vertex v1, v2; // in fact i can use v istead of both of them
			int din, dout;
			std::tie(din, v1) = queueF.PeekMin();
			std::tie(dout, v2) = queueB.PeekMin();
			if (mu <= din + dout) {
				muIsOpt = true;
				break;
			}
		}
		//emulate simple dijkstra
		uint32_t dis = get(distanceF, m_center) + get(distanceB, m_center);
		put(distanceF, t, dis);


		Vertex predecessor = m_center;
		Vertex current = get(predecessorB, predecessor);
		while(current != t) {
			put(predecessorF, current, predecessor);
			predecessor = current;
			current = get(predecessorB, current);
		}
	}
}

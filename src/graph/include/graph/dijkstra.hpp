#pragma once
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/static_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <graph/queue/SetQueue.hpp>
#include <graph/queue/HeapQueue.hpp>
#include <limits>

namespace graph
{
	template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
	          typename IndexMapTag, typename ColorMapTag, typename BundledVertexProperties,
	          typename BundledEdgeProperties>
	struct GenerateDijkstraGraph {};


	template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
	          typename IndexMapTag, typename ColorMapTag, typename... P1s, typename... P2s>
	struct GenerateDijkstraGraph<PredecessorMapTag, DisanceMapTag, WeightMapTag,
	                             IndexMapTag, ColorMapTag, Properties<P1s...>, Properties<P2s...>> {
		using type = StaticGraph<
			Properties<
				Property<PredecessorMapTag,
				         typename graph_traits<StaticGraph<Properties<>, Properties<>>>::vertex_descriptor>,
				Property<DisanceMapTag, uint32_t>,
				Property<ColorMapTag, boost::two_bit_color_type>,
				P1s...>,
			Properties<
				Property<WeightMapTag, uint32_t>,
				P2s...>>;
	};

	template <typename Graph>
	struct NoFieldsDijkstraVisitor {
		// This is invoked one each vertex of the graph when it is initialized.
		void initialize_vertex(const typename graph_traits<Graph>::vertex_descriptor&, const Graph&) {};

		// This is invoked on a vertex as it is popped from the queue.
		// This happens immediately before examine_edge() is invoked on each of the out - edges of vertex u.
		void examine_vertex(const typename graph_traits<Graph>::vertex_descriptor&, const Graph&) {};

		// This is invoked on every out - edge of each vertex after it is discovered.
		void examine_edge(const typename graph_traits<Graph>::edge_descriptor&, const Graph&) {};

		// This is invoked when a vertex is encountered for the first time.
		void discover_vertex(const typename graph_traits<Graph>::vertex_descriptor&, const Graph&) {};

		// Upon examination, if the following condition holds then the edge is relaxed(its distance is reduced), and this method is invoked.
		void edge_relaxed(const typename graph_traits<Graph>::edge_descriptor&, const Graph&) {};

		// Upon examination, if the edge is not relaxed(see above) then this method is invoked.
		void edge_not_relaxed(const typename graph_traits<Graph>::edge_descriptor&, const Graph&) {};

		// This invoked on a vertex after all of its out edges have been added to the search tree and
		// all of the adjacent vertices have been discovered(but before their out - edges have been examined).
		void finish_vertex(const typename graph_traits<Graph>::vertex_descriptor&, const Graph&) {};

		// A predicate which is invoked on every out - edge of each vertex to check if the algorithm should relax it
		bool should_relax(const typename graph_traits<Graph>::edge_descriptor&, const Graph&) {
			return true;
		};

		// A predicate  which is invoked after finish_vertex to check if the algorithm should continue
		bool should_continue() {
			return true;
		};

	};

	template <typename Graph>
	struct DefaultDijkstraVisitor : public NoFieldsDijkstraVisitor<Graph> {
		using QueueType = queue::FastHeapQueue<int, typename graph_traits<Graph>::vertex_descriptor>;

		DefaultDijkstraVisitor()
			: Queue() {}

		// structures
		QueueType Queue;

		void initializeQueue(Graph& graph) {
			Queue.Resize(num_vertices(graph));
			Queue.Clear();
		}
	};


	template <class Graph, class PredecessorMap, class DistanceMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor>
	void init_one_vertex(Graph& graph,
	                     const typename graph_traits<Graph>::vertex_descriptor& v,
	                     PredecessorMap& predecessor,
	                     DistanceMap& distance,
	                     IndexMap& index, ColorMap& color,
	                     DijkstraVisitor& visitor,
	                     typename graph_traits<Graph>::vertex_descriptor maxDistance = std::numeric_limits<typename graph_traits<Graph>::vertex_descriptor>::max() >> 2
	) {
		visitor.initialize_vertex(v, graph);
		put(distance, v, maxDistance);
		put(predecessor, v, v);
		put(color, v, boost::two_bit_color_type::two_bit_white);
	}

	template <class Graph, class DistanceMap, class IndexMap,
	          class ColorMap, class DijkstraVisitor, class Queue>
	void init_first_vertex(Graph& graph,
	                       const typename graph_traits<Graph>::vertex_descriptor& v,
	                       DistanceMap& distance,
	                       IndexMap& index, ColorMap& color,
	                       DijkstraVisitor& visitor, Queue& queue) {
		visitor.discover_vertex(v, graph);
		put(distance, v, 0);
		put(color, v, boost::two_bit_color_type::two_bit_green);
		queue.Insert(0, v);
	}

	template <class Graph, class PredecessorMap, class DistanceMap, class WeightMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor>
	bool dijkstra_iteration(Graph& graph,
	                        PredecessorMap& predecessor, DistanceMap& distance, WeightMap& weight,
	                        IndexMap& index, ColorMap& color, DijkstraVisitor& visitor) {
		using Vertex = typename graph_traits<Graph>::vertex_descriptor;
		auto& queue = visitor.Queue;
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
			}
			else {
				// Found same or worse distance
				visitor.edge_not_relaxed(edge, graph);
			}
		}

		// Teardown vertex
		put(color, v, boost::two_bit_color_type::two_bit_black);
		visitor.finish_vertex(v, graph);
		return visitor.should_continue();
	};


	template <class Graph, class PredecessorMap, class DistanceMap, class WeightMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor = DefaultDijkstraVisitor<Graph>>
	void dijkstra(Graph& graph,
	              const typename graph_traits<Graph>::vertex_descriptor& s,
	              PredecessorMap& predecessor, DistanceMap& distance, WeightMap& weight,
	              IndexMap& index, ColorMap& color) {
		DijkstraVisitor visitor;
		dijkstra(graph, s, predecessor, distance, weight, index, color, visitor);
	}

	template <class Graph, class PredecessorMap, class DistanceMap, class WeightMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor = DefaultDijkstraVisitor<Graph>>
	void dijkstra(Graph& graph,
	              const typename graph_traits<Graph>::vertex_descriptor& s,
	              PredecessorMap& predecessor, DistanceMap& distance, WeightMap& weight,
	              IndexMap& index, ColorMap& color, DijkstraVisitor& visitor) {
		using Vertex = typename graph_traits<Graph>::vertex_descriptor;
		// TODO 
		// - Use IndexMap instead of Vertex
		visitor.initializeQueue(graph);
		auto& queue = visitor.Queue;

		for (auto& v : graphUtil::Range(vertices(graph))) {
			init_one_vertex(graph, v, predecessor, distance, index, color, visitor, std::numeric_limits<Vertex>::max());
		}

		// Process start vertex
		init_first_vertex(graph, s, distance, index, color, visitor, queue);

		while (!queue.IsEmpty()) {
			auto allRight = dijkstra_iteration(graph, predecessor, distance, weight, index, color, visitor);
			if (!allRight)
				break;
		}
	};
}

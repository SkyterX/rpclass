#pragma once
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/static_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <graph/queue/DijkstraQueue.hpp>
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

	template <typename DistanceMap>
	constexpr typename DistanceMap::value_type InfinityDistance() {
		return std::numeric_limits<typename DistanceMap::value_type>::max();
	}

	template <typename Graph>
	struct IDijkstraVisitor {
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
	class LazyVertexInitializer {
	private:
		using VertexType = typename graph_traits<Graph>::vertex_descriptor;
		using IterationIdType = uint32_t;
		std::vector<IterationIdType> vertexIterationId;
		IterationIdType currentIterationId;
	public:

		void Initialize(Graph& graph) {
			auto verticesCount = num_vertices(graph);
			vertexIterationId.resize(verticesCount, 0);
			if (currentIterationId == std::numeric_limits<IterationIdType>::max()) {
				currentIterationId = 0;
				vertexIterationId.assign(verticesCount, 0);
			}
			++currentIterationId;
		}

		template <typename IndexMap>
		bool IsInitialized(const VertexType& v, const IndexMap& index) const {
			auto vertexIndex = get(index, v);
			return vertexIterationId[vertexIndex] == currentIterationId;
		}

		template <typename IndexMap>
		bool TryInitializeVertex(const VertexType& v, const IndexMap& index) {
			auto vertexIndex = get(index, v);
			if (vertexIterationId[vertexIndex] == currentIterationId)
				return false;
			vertexIterationId[vertexIndex] = currentIterationId;
			return true;
		}
	};

	template <typename Graph>
	struct DefaultDijkstraVisitor : public IDijkstraVisitor<Graph> {
		struct SharedDataStorage {
			using QueueType = queue::DijkstraQueue<
				uint32_t, 
				typename graph_traits<Graph>::vertex_descriptor,
				typename property_map<Graph, vertex_index_t>::type>;
			QueueType Queue;
			LazyVertexInitializer<Graph> VertexInitializer;
		};

		SharedDataStorage Stored;

		DefaultDijkstraVisitor()
			: Stored() {}

		void Initialize(Graph& graph) {
			auto verticesCount = num_vertices(graph);
			Stored.Queue.Resize(verticesCount);
			Stored.Queue.Clear();

			Stored.VertexInitializer.Initialize(graph);
		}
	};

	template <class Graph, class PredecessorMap, class DistanceMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor>
	void EnsureVertexInitialization(Graph& graph,
	                                const typename graph_traits<Graph>::vertex_descriptor& v,
	                                PredecessorMap& predecessor,
	                                DistanceMap& distance,
	                                IndexMap& index, ColorMap& color,
	                                DijkstraVisitor& visitor) {
		using DistanceType = typename DistanceMap::value_type;

		if (!visitor.Stored.VertexInitializer.TryInitializeVertex(v, index))
			return;

		visitor.initialize_vertex(v, graph);
		graph::put(distance, v, InfinityDistance<DistanceMap>());
		graph::put(predecessor, v, v);
		graph::put(color, v, boost::two_bit_color_type::two_bit_white);
	}

	template <class Graph, class PredecessorMap, class DistanceMap, class IndexMap,
	          class ColorMap, class DijkstraVisitor, class Queue>
	void init_first_vertex(Graph& graph,
	                       const typename graph_traits<Graph>::vertex_descriptor& v,
	                       PredecessorMap& predecessor,
	                       DistanceMap& distance,
	                       IndexMap& index, ColorMap& color,
	                       DijkstraVisitor& visitor, Queue& queue) {
		using DistanceType = typename DistanceMap::value_type;
		const DistanceType startDistance = 0;

		EnsureVertexInitialization(graph, v, predecessor, distance, index, color, visitor);
		visitor.discover_vertex(v, graph);
		graph::put(distance, v, startDistance);
		graph::put(color, v, boost::two_bit_color_type::two_bit_green);
		queue.Insert(startDistance, v, index);
	}

	template <class Graph, class PredecessorMap, class DistanceMap, class WeightMap,
	          class IndexMap, class ColorMap, class DijkstraVisitor>
	bool dijkstra_iteration(Graph& graph,
	                        PredecessorMap& predecessor, DistanceMap& distance, WeightMap& weight,
	                        IndexMap& index, ColorMap& color, DijkstraVisitor& visitor) {
		auto& queue = visitor.Stored.Queue;
		// Get vertex from queue
		auto topItem = queue.PeekMin();
		queue.DeleteMin();
		auto& v = topItem.Vertex;
		auto& vDistance = topItem.Distance;
		visitor.examine_vertex(v, graph);

		// Process edges	
		for (const auto& edge : graphUtil::Range(out_edges(v, graph))) {
			visitor.examine_edge(edge, graph);
			if (!visitor.should_relax(edge, graph))
				continue;
			// Get edge Properties
			auto to = target(edge, graph);
			EnsureVertexInitialization(graph, to, predecessor, distance, index, color, visitor);
			auto edgeWeight = graph::get(weight, edge);
			auto newDistance = vDistance + edgeWeight;
			auto toDistance = graph::get(distance, to);
			if (newDistance < toDistance) {
				// Found better distance -> update
				graph::put(distance, to, newDistance);
				graph::put(predecessor, to, v);
				if (graph::get(color, to) == boost::two_bit_color_type::two_bit_white) {
					// Vertex is new
					visitor.discover_vertex(to, graph);
					graph::put(color, to, boost::two_bit_color_type::two_bit_green);
					queue.Insert(newDistance, to, index);
				}
				else {
					queue.DecreaseKey(newDistance, to, index);
				}
				visitor.edge_relaxed(edge, graph);
			}
			else {
				// Found same or worse distance
				visitor.edge_not_relaxed(edge, graph);
			}
		}

		// Teardown vertex
		graph::put(color, v, boost::two_bit_color_type::two_bit_black);
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
		visitor.Initialize(graph);
		auto& queue = visitor.Stored.Queue;

		// Process start vertex		
		init_first_vertex(graph, s, predecessor, distance, index, color, visitor, queue);

		while (!queue.IsEmpty()) {
			auto allRight = dijkstra_iteration(graph, predecessor, distance, weight, index, color, visitor);
			if (!allRight)
				break;
		}
	};
}

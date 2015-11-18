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

	template <typename Graph, typename IndexMap,
	          typename DijkstraVisitorF, typename DijkstraVisitorB,
	          typename DistanceMapF, typename DistanceMapB,
	          typename ColorMapF, typename ColorMapB>
	class OptimalCriteriaTraker : public IDijkstraVisitor<Graph> {
		using Vertex = typename graph_traits<Graph>::vertex_descriptor;
		using DistanceType = typename DistanceMapF::value_type;
	public :
		void edge_relaxed(const typename graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
			Vertex to = target(edge, graph);
			if (!visitorF.Stored.VertexInitializer.IsInitialized(to, index) ||
				!visitorB.Stored.VertexInitializer.IsInitialized(to, index) ||
				get(colorF, to) == boost::two_bit_white ||
				get(colorB, to) == boost::two_bit_white)
				return;
			uint32_t right = get(distanceF, to) + get(distanceB, to);
			if (right < mu) {
				mu = right;
				transitNode = to;
			}
		};

		bool should_continue() {
			Vertex v1, v2;
			int din, dout;
			if (visitorF.Stored.Queue.IsEmpty() || visitorB.Stored.Queue.IsEmpty())
				return false;
			std::tie(din, v1) = visitorF.Stored.Queue.PeekMin();
			std::tie(dout, v2) = visitorB.Stored.Queue.PeekMin();
			return !(mu <= din + dout);
		};

		//can be easily changed to min queue iterator or something like it
		bool forward_iteration_is_next() {
			direction_flag_forward = !direction_flag_forward;
			return direction_flag_forward;
		}

		bool AnyPathFound() const {
			return mu < InfinityDistance<DistanceMapF>();
		}

		OptimalCriteriaTraker(const DijkstraVisitorF& visitorF, const DijkstraVisitorB& visitorB,
		                      DistanceMapF& distanceF, DistanceMapB& distanceB,
		                      ColorMapF& colorF, ColorMapB& colorB,
		                      const IndexMap& index)
			: visitorF(visitorF),
			  visitorB(visitorB),
			  index(index),
			  distanceF(distanceF),
			  distanceB(distanceB),
			  colorF(colorF),
			  colorB(colorB) {
			mu = InfinityDistance<DistanceMapF>();
			direction_flag_forward = false;
			transitNode = std::numeric_limits<Vertex>::max();
		}

		DistanceType mu;
		Vertex transitNode;
		bool direction_flag_forward;
		const DijkstraVisitorF& visitorF;
		const DijkstraVisitorB& visitorB;
		const IndexMap& index;
		DistanceMapF& distanceF;
		DistanceMapB& distanceB;
		ColorMapF& colorF;
		ColorMapB& colorB;
	};

	template <typename Graph, typename MainVisitor, typename AdditionalVisitor>
	class DijkstraVisitorCombinator : public IDijkstraVisitor<Graph> {
	public:
		void initialize_vertex(const typename graph_traits<Graph>::vertex_descriptor& v, Graph& g) {
			mainVisitor.initialize_vertex(v, g);
			additionalVisitor.initialize_vertex(v, g);
		};

		void examine_vertex(const typename graph_traits<Graph>::vertex_descriptor& v, Graph& g) {
			mainVisitor.examine_vertex(v, g);
			additionalVisitor.examine_vertex(v, g);
		};

		void examine_edge(const typename graph_traits<Graph>::edge_descriptor& e, Graph& g) {
			mainVisitor.examine_edge(e, g);
			additionalVisitor.examine_edge(e, g);
		};

		void discover_vertex(const typename graph_traits<Graph>::vertex_descriptor& v, Graph& g) {
			mainVisitor.discover_vertex(v, g);
			additionalVisitor.discover_vertex(v, g);
		};

		void edge_relaxed(const typename graph_traits<Graph>::edge_descriptor& e, Graph& g) {
			mainVisitor.edge_relaxed(e, g);
			additionalVisitor.edge_relaxed(e, g);
		};

		void edge_not_relaxed(const typename graph_traits<Graph>::edge_descriptor& e, Graph& g) {
			mainVisitor.edge_not_relaxed(e, g);
			additionalVisitor.edge_not_relaxed(e, g);
		};

		void finish_vertex(const typename graph_traits<Graph>::vertex_descriptor& v, Graph& g) {
			mainVisitor.finish_vertex(v, g);
			additionalVisitor.finish_vertex(v, g);
		};

		bool should_relax(const typename graph_traits<Graph>::edge_descriptor& e, Graph& g) {
			if (!mainVisitor.should_relax(e, g)) return false;
			return additionalVisitor.should_relax(e, g);
		};

		bool should_continue() {
			if (!mainVisitor.should_continue()) return false;
			return additionalVisitor.should_continue();
		};

		DijkstraVisitorCombinator(MainVisitor& mainV, AdditionalVisitor& additionalV)
			: mainVisitor(mainV),
			  additionalVisitor(additionalV),
			  Stored(mainV.Stored) {}

		MainVisitor& mainVisitor;
		AdditionalVisitor& additionalVisitor;
		typename MainVisitor::SharedDataStorage& Stored;
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
	                            IndexMap& index, ColorMapF& colorF, ColorMapB& colorB) {
		DijkstraVisitorF visitorF;
		DijkstraVisitorB visitorB;
		bidirectional_dijkstra(graph, s, t, predecessorF, predecessorB,
		                       distanceF, distanceB, weight, index, colorF, colorB, visitorF, visitorB);
	}


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
	                            DijkstraVisitorF& visitorF,
	                            DijkstraVisitorB& visitorB) {

		if (s == t) {
			put(distanceF, t, 0);
			put(predecessorF, t, t);
			return;
		}
		using Vertex = typename graph_traits<Graph>::vertex_descriptor;
		using Queue = queue::FastHeapQueue<int, Vertex>;
		using OptimalCriteriaTrakerType = OptimalCriteriaTraker<Graph, IndexMap, DijkstraVisitorF, DijkstraVisitorB, DistanceMapF, DistanceMapB, ColorMapF, ColorMapB>;
		auto invertedGraph = graph::ComplementGraph<Graph>(graph);

		visitorF.Initialize(graph);
		visitorB.Initialize(invertedGraph);
		auto& queueF = visitorF.Stored.Queue;
		auto& queueB = visitorB.Stored.Queue;

		// Process start vertex
		init_first_vertex(graph, s, predecessorF, distanceF, index, colorF, visitorF, queueF);
		init_first_vertex(invertedGraph, t, predecessorB, distanceB, index, colorB, visitorB, queueB);

		OptimalCriteriaTrakerType optTracker(visitorF, visitorB, distanceF, distanceB, colorF, colorB, index);

		DijkstraVisitorCombinator<Graph, DijkstraVisitorF, OptimalCriteriaTrakerType>
				bivisitorF(visitorF, optTracker);
		DijkstraVisitorCombinator<Graph, DijkstraVisitorB, OptimalCriteriaTrakerType>
				bivisitorB(visitorB, optTracker);

		while (!queueF.IsEmpty() && !queueB.IsEmpty()) {
			if (optTracker.forward_iteration_is_next()) {
				bool shouldContinue = dijkstra_iteration(graph, predecessorF, distanceF, weight, index, colorF, bivisitorF);
				if (!shouldContinue)
					break;
			}
			else {
				bool shouldContinue = dijkstra_iteration(invertedGraph, predecessorB, distanceB, weight, index, colorB, bivisitorB);
				if (!shouldContinue)
					break;
			}
		}

		if (!optTracker.AnyPathFound())
			return;

		//emulate simple dijkstra
		uint32_t dis = optTracker.mu;
		put(distanceF, t, dis);
		
		Vertex predecessor = optTracker.transitNode;
		Vertex current = get(predecessorB, predecessor);
		while (current != t) {
			put(predecessorF, current, predecessor);
			predecessor = current;
			current = get(predecessorB, current);
		}
	}
}

#pragma once

#include <iostream>
#include <graph/dijkstra.hpp>
#include <graph/dynamic_graph.hpp>
#include <boost/graph/two_bit_color_map.hpp>
#include <graph/bidirectional_dijkstra.hpp>

namespace ch {

    template <typename Graph>
    using DefaultCHVisitor = graph::DefaultDijkstraVisitor<Graph>;
    enum class DirectionBit:char {
        both=0,
        forward=1,
        backward=2,        
    };

template <typename PredecessorFMapTag, typename PredecessorBMapTag, typename DisanceMapFTag, class DisanceMapBTag,
    typename WeightMapTag, typename IndexMapTag, typename ColorFMapTag, typename ColorBMapTag, typename UnPackMapTag,
    typename VertexOrderMapTag, typename DirectionMapTag,
    typename VP, typename EP, typename GP>
struct GenerateCHGraph {};

template <typename PredecessorFMapTag, typename PredecessorBMapTag, typename DisanceMapFTag, typename DisanceMapBTag,
    typename WeightMapTag, typename IndexMapTag, typename ColorFMapTag, typename ColorBMapTag, typename UnPackMapTag,
    typename VertexOrderMapTag, typename DirectionMapTag,
    typename... P1s, typename... P2s, typename ... P3s>
struct GenerateCHGraph<PredecessorFMapTag, PredecessorBMapTag, DisanceMapFTag, DisanceMapBTag, WeightMapTag,
    IndexMapTag, ColorFMapTag, ColorBMapTag, UnPackMapTag, VertexOrderMapTag, DirectionMapTag,
    graph::Properties<P1s...>, graph::Properties<P2s...>, graph::Properties<P3s... >> {
    using EmptyGraph = graph::DynamicGraph <
        graph::Properties<>, graph::Properties<>, graph::Properties<> >;

    using vertex_descriptor =
        typename graph::graph_traits<EmptyGraph>::vertex_descriptor;
    using vertices_size_type =
        typename graph::graph_traits<EmptyGraph>::vertices_size_type;

    using type = graph::DynamicGraph <
        graph::Properties<
            graph::Property<PredecessorFMapTag, vertex_descriptor>,
			graph::Property<PredecessorBMapTag, vertex_descriptor>,
            graph::Property<DisanceMapFTag, uint32_t>,
            graph::Property<DisanceMapBTag, uint32_t>,
            graph::Property<ColorFMapTag, boost::two_bit_color_type>,
			graph::Property<ColorBMapTag, boost::two_bit_color_type>,
            graph::Property<VertexOrderMapTag, vertices_size_type>,            
            P1s...>,
        graph::Properties<
            graph::Property<WeightMapTag, uint32_t>,
            graph::Property<UnPackMapTag, vertex_descriptor>,
            graph::Property<DirectionMapTag, DirectionBit>,
            P2s...>,
        graph::Properties<P3s...>
    >;
};

template <typename Graph>
class ShortCutOrderStrategy {
public:
    typename graph::graph_traits<Graph>::vertex_descriptor
        next(Graph& graph) {};
};

template <typename Graph>
class HLOrderStrategy {
public:

    typename graph::graph_traits<Graph>::vertex_descriptor
        next(Graph& graph) {};
};

template <typename Graph, typename VertexOrderMap>
class DumbOrderStrategy {
private:
	using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
	using VertexIterator = typename graph::graph_traits<Graph>::vertex_iterator;
	VertexIterator currentIterator, endIterator;
	size_t order;
public:

	DumbOrderStrategy(Graph& graph) {
		std::tie(currentIterator, endIterator) = vertices(graph);
		order = 0;
	}

	Vertex
		next(Graph& graph) {

		if (currentIterator != endIterator) {
			auto v = *currentIterator;
			++currentIterator;
			++order;
			return v;
		}
		return graph.null_vertex();
	};
};

template <typename Graph, typename OrderMap, typename DirectionMap>
struct CHPreprocessDijkstraVisitor : public graph::DefaultDijkstraVisitor<Graph> {
	using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;

	CHPreprocessDijkstraVisitor(Vertex& ignoredVertex, Vertex& finishVertex, size_t hopCount, OrderMap& order, DirectionMap& direction)
	: ignoredVertex(ignoredVertex), finishVertex(finishVertex), order(order), direction(direction), maxHopCount(hopCount){
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

template <typename Graph, typename OrderMap, typename DirectionMap>
struct CHQueryVisitor : public graph::DefaultDijkstraVisitor<Graph> {
	using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;

	CHQueryVisitor(OrderMap& order, DirectionMap& direction, DirectionBit ignoredDirection)
		: directionBit(ignoredDirection), order(order), direction(direction) {}

	bool should_relax(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
		auto to = target(edge, graph);
		auto from = source(edge, graph);

		if (get(direction, edge) != directionBit) {
			if (directionBit != DirectionBit::backward && get(order, to) > get(order, from)) {
				std::cout << "Relaxing edge forward: " << from - 1 << " " << to - 1 << std::endl;
				return true;
			}
			if (directionBit != DirectionBit::forward && get(order, from) < get(order, to)) {
				std::cout << "Relaxing edge backward: " << from - 1 << " " << to - 1 << std::endl;
				return true;
			}
		}
		return false;
	}

private:
	DirectionBit directionBit;
	OrderMap order;
	DirectionMap direction;
};


// uses dijkstra, therefore should have at least all property maps used by dijkstra
template <typename Graph, typename PredecessorMap, typename DistanceMap,
    typename WeightMap, typename IndexMap, typename ColorMap,typename UnPackMap,
    typename VertexOrderMap, typename DirectionMap,
    typename OrderStrategy = ShortCutOrderStrategy<Graph>>
void ch_preprocess(Graph& graph, PredecessorMap& predecessor, DistanceMap& distance,
        WeightMap& weight, IndexMap& index, ColorMap& color, UnPackMap& unpack,
        VertexOrderMap& order, DirectionMap& direction, size_t dijLimit,
        OrderStrategy&& strategy = OrderStrategy()) {
	using namespace std;
	using namespace graphUtil;
	using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;

	auto curVert = strategy.next(graph);

	for (const auto& vertex: Range(vertices(graph))) {
		graph::put(order, vertex, numeric_limits<size_t>::max());
	}

	size_t curOrder = 0;
	while (curVert != graph.null_vertex()) {
		graph::put(order, curVert, curOrder++);

		vector<tuple<Vertex, Vertex, size_t>> shortCuts;

		for (const auto& out_e: Range(out_edges(curVert, graph))) {
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
				
				auto out_v = target(out_e, graph);
				auto in_v = target(in_e, graph);
				
				auto shortCutLength = get(weight, out_e) + get(weight, in_e);

				CHPreprocessDijkstraVisitor<Graph, VertexOrderMap, DirectionMap> visitor(curVert, out_v, dijLimit, order, direction);
				graph::dijkstra(graph, in_v, predecessor, distance, weight, index, color, visitor);

				if (get(color, out_v) == boost::two_bit_black) {
					if (get(distance, out_v) < shortCutLength)
						continue;
				}

				cout << "Shortcut added: " << in_v + 1 << " to " << out_v + 1 << "with length " << shortCutLength << endl;
				shortCuts.push_back(make_tuple(in_v, out_v, shortCutLength));

			}
		}

		for (auto shortCut : shortCuts) {
			auto pr = add_edge(get<0>(shortCut), get<1>(shortCut), graph);
			graph::put(weight, pr.first, get<2>(shortCut));
			graph::put(direction, pr.first, DirectionBit::forward);
			graph::put(unpack, pr.first, curVert);

			auto pr1 = add_edge(get<1>(shortCut), get<0>(shortCut), graph);
			graph::put(weight, pr1.first, get<2>(shortCut));
			graph::put(direction, pr1.first, DirectionBit::backward);
			graph::put(unpack, pr1.first, curVert);
		}

		curVert = strategy.next(graph);
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
    void ch_query(Graph& graph, 
        const typename graph::graph_traits<Graph>::vertex_descriptor& s,
        const typename graph::graph_traits<Graph>::vertex_descriptor& t,
        PredecessorFMap& predecessorF, PredecessorBMap& predecessorB, DistanceFMap& distanceF, DistanceBMap& distanceB,
        WeightMap& weight, IndexMap& index, ColorFMap& colorF, ColorBMap& colorB, UnPackMap& unpack,
        VertexOrderMap& order, DirectionMap& direction, CHVisitor&& visitor = CHVisitor()) {
        //IncreaseWeightOfIncommingEdgeVisitor<Graph,DirectionMap,WeightMap> dijVisitor(direction, weight);
        //graph::dijkstra(graph, s, predecessor, distanceF, weight, index, color, dijVisitor);

	auto chFVisitor = CHQueryVisitor<Graph, VertexOrderMap, DirectionMap>(order, direction, DirectionBit::backward);
	auto chBVisitor = CHQueryVisitor<Graph, VertexOrderMap, DirectionMap>(order, direction, DirectionBit::forward);

	graph::bidirectional_dijkstra(graph, s, t, predecessorF, predecessorB, distanceF, distanceB, weight, index, colorF, colorB, chFVisitor, chBVisitor);

    };
};
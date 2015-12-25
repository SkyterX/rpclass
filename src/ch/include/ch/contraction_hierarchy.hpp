#pragma once

#include <random>
#include <map>
#include <iostream>
#include <graph/dijkstra.hpp>
#include <graph/dynamic_graph.hpp>
#include <boost/graph/two_bit_color_map.hpp>
#include <graph/bidirectional_dijkstra.hpp>
#include <graph/detail/IncidenceGraph.hpp>

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

template <typename Graph>
class RandomStrategy {
	using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
	using VertexOrder = std::vector<Vertex>;
	using VertexIterator = typename VertexOrder::iterator;
	VertexOrder order;
	VertexIterator current;

public:

	RandomStrategy(Graph& graph) {
		order = graphUtil::AsArray(graphUtil::Range(vertices(graph)));
		std::mt19937 generator(3561237589);
		std::shuffle(order.begin(), order.end(), generator);
//		std::cout << "Order : ";
//		for(auto &v : order) {
//			std::cout << v+1 << " ";
//		}
//		std::cout << std::endl;
		current = order.begin();
	}

	typename graph::graph_traits<Graph>::vertex_descriptor
		next(Graph& graph) {
		if(current != order.end()) {
			auto v = *current;
			++current;
			return v;
		}
		return graph.null_vertex();
	};
};

template <typename Graph>
class DumbOrderStrategy {
private:
	using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
	using VertexIterator = typename graph::graph_traits<Graph>::vertex_iterator;
	VertexIterator currentIterator, endIterator;
public:

	DumbOrderStrategy(Graph& graph) {
		std::tie(currentIterator, endIterator) = vertices(graph);
	}

	Vertex
		next(Graph& graph) {

		if (currentIterator != endIterator) {
			auto v = *currentIterator;
			++currentIterator;
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
		: ignoredDirection(ignoredDirection), order(order), direction(direction) {}

	bool should_relax(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
		auto to = target(edge, graph);
		auto from = source(edge, graph);
//		std::cout << "\tShould " << (ignoredDirection == DirectionBit::backward ? "forward" : "backward") << " search relax edge "
//			<< from + 1 << " " << to + 1 << " "
//			<< (get(direction, edge) == DirectionBit::backward ? "backward" : "forward") << std::endl;
		if (get(direction, edge) != ignoredDirection) {
			if (ignoredDirection != DirectionBit::backward && get(order, to) > get(order, from)) {
//				std::cout << "\tRelaxing edge backward: " << to + 1 << " " << from + 1 << std::endl;
				return true;
			}
			if (ignoredDirection != DirectionBit::forward && get(order, to) > get(order, from)) {
//				std::cout << "\tRelaxing edge forward: " << from + 1 << " " << to + 1 << std::endl;
				return true;
			}
		}
		return false;
	}

private:
	DirectionBit ignoredDirection;
	OrderMap order;
	DirectionMap direction;
};

	template <typename Graph, typename IndexMap,
	          typename DijkstraVisitorF, typename DijkstraVisitorB,
	          typename DistanceMapF, typename DistanceMapB,
	          typename ColorMapF, typename ColorMapB>
	class CHOptimalCriteriaTraker : public graph::OptimalCriteriaTraker<
				Graph, IndexMap,
				DijkstraVisitorF, DijkstraVisitorB,
				DistanceMapF, DistanceMapB,
				ColorMapF, ColorMapB> {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using DistanceType = typename DistanceMapF::value_type;
	public:

		CHOptimalCriteriaTraker(
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

		bool should_continue() {
			if (this->visitorF.Stored.Queue.IsEmpty() || this->visitorB.Stored.Queue.IsEmpty())
				return true;
			auto& topItem = this->direction_flag_forward
				? this->visitorF.Stored.Queue.PeekMin()
				: this->visitorB.Stored.Queue.PeekMin();
			return !(this->mu <= topItem.Distance);
		}
	};


// uses dijkstra, therefore should have at least all property maps used by dijkstra
template <typename Graph, typename PredecessorMap, typename DistanceMap,
    typename WeightMap, typename IndexMap, typename ColorMap,typename UnPackMap,
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

	for (const auto& vertex: Range(vertices(graph))) {
		graph::put(order, vertex, numeric_limits<size_t>::max());
	}

	size_t curOrder = 0;
	int counter = 0;
	while (curVert != graph.null_vertex()) {
		graph::put(order, curVert, curOrder++);
		++counter;
		if(counter % 1000 == 0)
			cout << "Processing "<< counter << " vertex Id : " << curVert + 1 << " Degree : " << out_degree(curVert, graph) <<endl;

		map<pair<Vertex, Vertex>, size_t> shortCuts;
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
				if(it == shortCuts.end() || it->second > shortCutLength)
					shortCuts[shortCutKey] = shortCutLength;

			}
		}
		//remove edges
		for (const auto& edge : AsArray(Range(out_edges(curVert, graph)))) {
			auto to = target(edge, graph);
			auto from = source(edge, graph);
			if(get(order, to) <= get(order, from)) {
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
    void ch_query(Graph& originalGraph, 
        const typename graph::graph_traits<Graph>::vertex_descriptor& s,
        const typename graph::graph_traits<Graph>::vertex_descriptor& t,
        PredecessorFMap& predecessorF, PredecessorBMap& predecessorB, DistanceFMap& distanceF, DistanceBMap& distanceB,
        WeightMap& weight, IndexMap& index, ColorFMap& colorF, ColorBMap& colorB, UnPackMap& unpack,
        VertexOrderMap& order, DirectionMap& direction, CHVisitor&& visitor = CHVisitor()) {
        //IncreaseWeightOfIncommingEdgeVisitor<Graph,DirectionMap,WeightMap> dijVisitor(direction, weight);
        //graph::dijkstra(graph, s, predecessor, distanceF, weight, index, color, dijVisitor);
	using VisitorFType = CHQueryVisitor<Graph, VertexOrderMap, DirectionMap>;
	using VisitorBType = CHQueryVisitor<Graph, VertexOrderMap, DirectionMap>;
	using TrackerType = CHOptimalCriteriaTraker<Graph, IndexMap, VisitorFType, VisitorBType, DistanceFMap, DistanceBMap, ColorFMap, ColorBMap>;
	auto graph = graph::IncidenceGraph<Graph>(originalGraph);

	auto chFVisitor = VisitorFType(order, direction, DirectionBit::backward);
	auto chBVisitor = VisitorBType(order, direction, DirectionBit::forward);

	graph::fancy_bidirectional_dijkstra<TrackerType>(graph, graph, s, t, predecessorF, predecessorB, distanceF, distanceB, weight, index, colorF, colorB, chFVisitor, chBVisitor);
//	graph::dijkstra(graph, s, predecessorF, distanceF, weight, index, colorF, chFVisitor);

    };
};
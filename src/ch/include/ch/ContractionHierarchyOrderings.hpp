#pragma once

#include <tuple>
#include <queue>
#include <vector>
#include <unordered_set>
#include <random>
#include <graph/graph.hpp>
#include <unordered_map>
#include <graph/detail/util/Collection.hpp>
#include <graph/queue/HeapQueue.hpp>
#include <graph/queue/SegmentTreeQueue.hpp>
#include <graph/Hash.hpp>
#include <ch/ContractionHierarchyUtils.hpp>

namespace ch
{
	template <typename Graph>
	class ShortCutOrderStrategy {
	public:
		typename graph::graph_traits<Graph>::vertex_descriptor
		next(Graph& graph) {};
	};

	template <typename Graph, typename PriorityCalculator>
	class AbstractPriorityOrderStrategy {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using Priority = typename PriorityCalculator::value_type;
		using QueueItemType = graph::queue::QueueItem<Priority, Vertex>;
		using QueueType = graph::queue::SegmentTreeQueue<Priority, Vertex>;

		bool isInitialized;
		QueueType queue;
		std::queue<Vertex> updateQueue;
		std::unordered_set<Vertex> usedVertices;
		PriorityCalculator priorityCalculator;
		Vertex previousVertex;
	public:

		AbstractPriorityOrderStrategy(Graph& g, PriorityCalculator& priorityCalculator)
			: isInitialized(false),
			  queue(graph::num_vertices(g)),
			  priorityCalculator(priorityCalculator) { }

		void Initialize(Graph& graph) {
			using namespace graphUtil;
			using namespace graph;

			for (auto& v : Range(vertices(graph))) {
				auto p = priorityCalculator(graph.null_vertex(), v, graph);
				queue.Insert(p, v);
			}
			isInitialized = true;
		}

		void UpdatePreviousVertex(Graph& graph) {
			while (!updateQueue.empty()) {
				auto v = updateQueue.front();
				updateQueue.pop();
				if (usedVertices.find(v) != usedVertices.end())
					continue;
				auto p = priorityCalculator(previousVertex, v, graph);
				queue.DecreaseKey(p, v, p);
			}
		}

		Vertex next(Graph& graph) {
			using namespace graphUtil;
			using namespace graph;

			if (!isInitialized) {
				Initialize(graph);
			}
			if (queue.IsEmpty()) {
				return graph.null_vertex();
			}

			UpdatePreviousVertex(graph);

			Vertex v;
			Priority p;
			std::tie(p, v) = queue.PeekMin();
			queue.DeleteMin();

			for (auto& to : Range(adjacent_vertices(v, graph))) {
				updateQueue.push(to);
			}
			usedVertices.insert(v);
			previousVertex = v;
			return v;
		};
	};

	template <typename Graph>
	class VertexDegreeCalculator {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using DegreeType = typename graph::graph_traits<Graph>::degree_size_type;
	public:
		using value_type = DegreeType;

		DegreeType operator()(const Vertex& previousVertex, const Vertex& v, Graph& g) const {
			return graph::out_degree(v, g);
		}
	};

	template <typename Graph>
	using OnlineVertexDegreeOrderStrategy = AbstractPriorityOrderStrategy<Graph, VertexDegreeCalculator<Graph>>;

	template <typename Graph>
	OnlineVertexDegreeOrderStrategy<Graph> GenerateOnlineVertexDegreeOrderStrategy(Graph& graph) {
		return OnlineVertexDegreeOrderStrategy<Graph>(graph, VertexDegreeCalculator<Graph>());
	}

	template <typename Graph, typename PredecessorFMap, typename PredecessorBMap, typename DistanceFMap,
	          typename DistanceBMap, typename WeightMap, typename IndexMap, typename ColorFMap, typename ColorBMap,
	          typename VertexOrderMap, typename DirectionMap>
	class HLCalculator {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using DegreeType = typename graph::graph_traits<Graph>::degree_size_type;
		using DijkstraVisitorType = CHPreprocessDijkstraVisitor<Graph, VertexOrderMap, DirectionMap>;
		using TrackerType = CHPreprocessOptimalCriteriaTraker<Graph, IndexMap, DijkstraVisitorType, DijkstraVisitorType, DistanceFMap, DistanceBMap, ColorFMap, ColorBMap>;
		using Edge = std::pair<Vertex, Vertex>;
		std::unordered_map<Vertex, int> L; // can become properties
		std::unordered_map<Edge, int> h;
		DirectionMap& direction;
		PredecessorFMap& predecessorF;
		PredecessorBMap& predecessorB;
		DistanceFMap& distanceF;
		DistanceBMap& distanceB;
		WeightMap& weight;
		IndexMap& index;
		ColorFMap& colorF;
		ColorBMap& colorB;
		VertexOrderMap& order;
		size_t dijLimit;


		template <typename Key, typename Value, typename MapType>
		Value GetOrDefault(Key key, Value defaultValue, const MapType& map) {
			Value lx = defaultValue;
			auto search = map.find(key);
			if (search != map.end()) {
				lx = search->second;
			}
			return lx;
		}

		double calculatePriority(const Vertex& x, Graph& graph) {
			double lx = GetOrDefault(x, 0, L);
			double ordA = 0;
			double ordD = 0;//out_degree(x, graph);
			double sumhA = 0;
			double sumhD = 0;
			// find incident edges for sumhD
			//			for (const auto& e : graphUtil::Range(out_edges(x, graph))) {
			//				if (get(direction, e) != DirectionBit::backward) { // i.e. forward
			//					sumhD += initOrGet(std::make_pair(x, target(e, graph)), 1, h);
			//				}
			//				if (get(direction, e) != DirectionBit::forward) { // i.e. backward
			//					sumhD += initOrGet(std::make_pair(target(e, graph), x), 1, h);
			//				}
			//			}
			// find A set
			auto curVert = x;
			std::map<std::pair<Vertex, Vertex>, size_t> shortCuts;
			for (const auto& out_e : graphUtil::Range(out_edges(curVert, graph))) {
				if (get(direction, out_e) == DirectionBit::backward) {
					continue;
				}
				for (const auto& in_e : graphUtil::Range(out_edges(curVert, graph))) {
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
					auto shortCutKey = std::make_pair(in_v, out_v);
					auto it = shortCuts.find(shortCutKey);
					if (it == shortCuts.end() || it->second > shortCutLength) {

						// calc ordA
						ordA++;
						// calc h(shortcut)
						int h_in_edge = GetOrDefault(std::make_pair(in_v, curVert), 1, h);
						int h_out_edge = GetOrDefault(std::make_pair(curVert, out_v), 1, h);
						int h_shortcut = h_in_edge + h_out_edge;
						h[std::make_pair(in_v, out_v)] = h_shortcut;
						sumhA += h_shortcut;

						shortCuts[shortCutKey] = shortCutLength;
					}

				}
			}

			// calc D set
			for (const auto& edge : AsArray(graphUtil::Range(out_edges(curVert, graph)))) {
				auto to = target(edge, graph);
				auto from = source(edge, graph);
				if (get(order, to) <= get(order, from)) {
					// calc ordD
					ordD++;
					// calc h(d)
					int h_in_edge = GetOrDefault(std::make_pair(from, curVert), 1, h);
					int h_out_edge = GetOrDefault(std::make_pair(curVert, to), 1, h);
					int h_shortcut = h_in_edge + h_out_edge;
					h[std::make_pair(from, to)] = h_shortcut;
					sumhD += h_shortcut;
				}
			}


			//			if (sumhD == 0 || ordD == 0) {
			//				std::cout << "Exception!!! There must not be a lonely vertices" << std::endl;
			//				throw 20;
			//			}
			double delimOrd = 0;
			if (ordD != 0) {
				delimOrd = (static_cast<double>(ordA) / ordD);
			}
			double delimSumH = 0;
			if (sumhD != 0) {
				delimSumH = (static_cast<double>(sumhA) / sumhD);
			}
			return lx + delimOrd + delimSumH;
		}


	public:
		using value_type = DegreeType;

		HLCalculator(Graph& graph,
		             PredecessorFMap& predecessorF, PredecessorBMap& predecessorB,
		             DistanceFMap& distanceF, DistanceBMap& distanceB,
		             WeightMap& weight, IndexMap& index,
		             ColorFMap& colorF, ColorBMap& colorB,
		             DirectionMap& direction, VertexOrderMap& order, size_t dijLimit)
			: direction(direction), predecessorF(predecessorF), predecessorB(predecessorB),
			  distanceF(distanceF), distanceB(distanceB), weight(weight), index(index),
			  colorF(colorF), colorB(colorB), order(order), dijLimit(dijLimit) {}

		DegreeType operator()(const Vertex& previousVertex, const Vertex& v, Graph& graph) {
			if (previousVertex != graph.null_vertex()) {
				//при  контракции x все смежные y l(y):=max{l(y), l(x)+1}
				int lx = GetOrDefault(previousVertex, 0, L);
				for (const auto& e : graphUtil::Range(out_edges(previousVertex, graph))) {
					auto y = target(e, graph);
					if (GetOrDefault(y, 0, L) < lx + 1) {
						L[y] = lx + 1;
					}
				};
			}
			return calculatePriority(v, graph);
		}
	};


	template <typename Graph, typename PredecessorFMap, typename PredecessorBMap, typename DistanceFMap,
	          typename DistanceBMap, typename WeightMap, typename IndexMap, typename ColorFMap, typename ColorBMap,
	          typename VertexOrderMap, typename DirectionMap>
	using HLOrderStrategy = AbstractPriorityOrderStrategy<
		Graph, HLCalculator<
			Graph, PredecessorFMap, PredecessorBMap,
			DistanceFMap, DistanceBMap, WeightMap, IndexMap, ColorFMap, ColorBMap, VertexOrderMap, DirectionMap>>;

	template <typename Graph, typename PredecessorFMap, typename PredecessorBMap, typename DistanceFMap,
	          typename DistanceBMap, typename WeightMap, typename IndexMap, typename ColorFMap, typename ColorBMap,
	          typename VertexOrderMap, typename DirectionMap>
	HLOrderStrategy<Graph, PredecessorFMap, PredecessorBMap, DistanceFMap,
	                DistanceBMap, WeightMap, IndexMap, ColorFMap, ColorBMap,
	                VertexOrderMap, DirectionMap> GenerateHLOrderStrategy(
		Graph& graph,
		PredecessorFMap& predecessorF, PredecessorBMap& predecessorB,
		DistanceFMap& distanceF, DistanceBMap& distanceB,
		WeightMap& weight, IndexMap& index,
		ColorFMap& colorF, ColorBMap& colorB,
		DirectionMap& direction, VertexOrderMap& oreder, size_t dijLimit) {
		return HLOrderStrategy<Graph, PredecessorFMap, PredecessorBMap, DistanceFMap,
		                       DistanceBMap, WeightMap, IndexMap, ColorFMap, ColorBMap,
		                       VertexOrderMap, DirectionMap>(
			graph,
			HLCalculator<Graph, PredecessorFMap, PredecessorBMap, DistanceFMap,
			             DistanceBMap, WeightMap, IndexMap, ColorFMap, ColorBMap,
			             VertexOrderMap, DirectionMap>(
				graph, predecessorF, predecessorB, distanceF,
				distanceB, weight, index, colorF, colorB, direction, oreder, dijLimit));
	}


	template <typename Graph, typename PredecessorFMap, typename PredecessorBMap, typename DistanceFMap,
		typename DistanceBMap, typename WeightMap, typename IndexMap, typename ColorFMap, typename ColorBMap,
		typename VertexOrderMap, typename DirectionMap>
	class OldHLOrderStrategy {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using DijkstraVisitorType = CHPreprocessDijkstraVisitor<Graph, VertexOrderMap, DirectionMap>;
		using TrackerType = CHPreprocessOptimalCriteriaTraker<Graph, IndexMap, DijkstraVisitorType, DijkstraVisitorType, DistanceFMap, DistanceBMap, ColorFMap, ColorBMap>;
		using Edge = std::pair<Vertex, Vertex>;
		using QueueItemType = graph::queue::QueueItem<double, Vertex>;
		using Queue = std::priority_queue<QueueItemType, std::vector<QueueItemType>, std::less<QueueItemType>>;
		std::unordered_map<Vertex, int> L; // can become properties
		std::unordered_map<Edge, int> h;
		Queue queue;
		DirectionMap& direction;
		PredecessorFMap& predecessorF;
		PredecessorBMap& predecessorB;
		DistanceFMap& distanceF;
		DistanceBMap& distanceB;
		WeightMap& weight;
		IndexMap& index;
		ColorFMap& colorF;
		ColorBMap& colorB;
		VertexOrderMap& order;
		size_t dijLimit;
		bool isInitialized;
		uint32_t itCount = 0;
		uint32_t maxGLCount = 0;

		template <typename Tkey, typename Tval, typename TMap>
		Tval initOrGet(Tkey key, Tval defaultValue, TMap& map) {
			Tval lx = defaultValue;
			auto search = map.find(key);
			if (search != map.end()) {
				lx = search->second;
			}
			return lx;
		}

		double calculatePriority(const Vertex& x, Graph& graph) {
			double lx = initOrGet(x, 0, L);
			double ordA = 0;
			double ordD = 0;//out_degree(x, graph);
			double sumhA = 0;
			double sumhD = 0;
			// find incident edges for sumhD
			//			for (const auto& e : graphUtil::Range(out_edges(x, graph))) {
			//				if (get(direction, e) != DirectionBit::backward) { // i.e. forward
			//					sumhD += initOrGet(std::make_pair(x, target(e, graph)), 1, h);
			//				}
			//				if (get(direction, e) != DirectionBit::forward) { // i.e. backward
			//					sumhD += initOrGet(std::make_pair(target(e, graph), x), 1, h);
			//				}
			//			}
			// find A set
			auto curVert = x;
			std::map<std::pair<Vertex, Vertex>, size_t> shortCuts;
			for (const auto& out_e : graphUtil::Range(out_edges(curVert, graph))) {
				if (get(direction, out_e) == DirectionBit::backward) {
					continue;
				}
				for (const auto& in_e : graphUtil::Range(out_edges(curVert, graph))) {
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

						// calc ordA
						ordA++;
						// calc h(shortcut)
						int h_in_edge = initOrGet(make_pair(in_v, curVert), 1, h);
						int h_out_edge = initOrGet(make_pair(curVert, out_v), 1, h);
						int h_shortcut = h_in_edge + h_out_edge;
						h[make_pair(in_v, out_v)] = h_shortcut;
						sumhA += h_shortcut;

						shortCuts[shortCutKey] = shortCutLength;
					}

				}
			}

			// calc D set
			for (const auto& edge : AsArray(graphUtil::Range(out_edges(curVert, graph)))) {
				auto to = target(edge, graph);
				auto from = source(edge, graph);
				if (get(order, to) <= get(order, from)) {
					// calc ordD
					ordD++;
					// calc h(d)
					int h_in_edge = initOrGet(make_pair(from, curVert), 1, h);
					int h_out_edge = initOrGet(make_pair(curVert, to), 1, h);
					int h_shortcut = h_in_edge + h_out_edge;
					h[make_pair(from, to)] = h_shortcut;
					sumhD += h_shortcut;
				}
			}



			//			if (sumhD == 0 || ordD == 0) {
			//				std::cout << "Exception!!! There must not be a lonely vertices" << std::endl;
			//				throw 20;
			//			}
			double delimOrd = 0;
			if (ordD != 0) {
				delimOrd = (static_cast<double>(ordA) / ordD);
			}
			double delimSumH = 0;
			if (sumhD != 0) {
				delimSumH = (static_cast<double>(sumhA) / sumhD);
			}
			return lx + delimOrd + delimSumH;
		}

		void Init(Graph& graph) {
			queue = Queue();
			L = std::unordered_map<Vertex, int>(); // can become properties
			h = std::unordered_map<Edge, int>();
			// init h = 1 // do not want to init so initOrGet method
			// init L = 0

			for (const auto& v : graphUtil::Range(vertices(graph))) {
				double priority = calculatePriority(v, graph);
				queue.push(QueueItemType(priority, v));
			}
			std::cout << "Initialization done" << std::endl;
		}

	public:

		OldHLOrderStrategy(Graph& graph,
			PredecessorFMap& predecessorF, PredecessorBMap& predecessorB,
			DistanceFMap& distanceF, DistanceBMap& distanceB,
			WeightMap& weight, IndexMap& index,
			ColorFMap& colorF, ColorBMap& colorB,
			DirectionMap& direction, VertexOrderMap& order, size_t dijLimit)
			:
			direction(direction), predecessorF(predecessorF), predecessorB(predecessorB),
			distanceF(distanceF), distanceB(distanceB), weight(weight), index(index),
			colorF(colorF), colorB(colorB), dijLimit(dijLimit), order(order) {
			isInitialized = false;
		}

		typename graph::graph_traits<Graph>::vertex_descriptor
			next(Graph& graph) {

			if (!isInitialized) {
				isInitialized = true;
				Init(graph);
			}

			if (queue.empty()) {
				return graph.null_vertex();
			}

			// take vertex with least priority
			QueueItemType queueItem = queue.top();
			Vertex return_v = queueItem.Data();
			queue.pop();

			uint32_t goodLuckCount = 1;

			do {
				// is it still least?
				auto newPriority = calculatePriority(return_v, graph);
				//				if (newPriority < priority) {
				//					std::cout << "new priority can't become less" << std::endl;
				//					throw 20;
				//				}
				// compare with second
				QueueItemType secondQueueItem = queue.top();
				auto nextVertex = secondQueueItem.Data();
				auto nextPriority = calculatePriority(nextVertex, graph);

				if (nextPriority >= newPriority) {
					break;
				}
				else { // least priority Vertex is a lie
					   //we should take our luck with next one
					queue.pop();
					queue.push(QueueItemType(newPriority, return_v));
					return_v = nextVertex;
					goodLuckCount++;
				}
			} while (true);

			//			maxGLCount = std::max(goodLuckCount, maxGLCount);
			//			if (itCount++ % 250 == 0) {
			//				std::cout << "\t Max Good luck on " << maxGLCount << std::endl;
			//				maxGLCount = 0;
			//			}

			//при  контракции x все смежные y l(y):=max{l(y), l(x)+1}
			int lx = initOrGet(return_v, 0, L);
			for (const auto& e : graphUtil::Range(out_edges(return_v, graph))) {
				auto y = target(e, graph);
				if (initOrGet(y, 0, L) < lx + 1) {
					L[y] = lx + 1;
				}
			};
			return return_v;
		};
	};


	template <typename Graph, typename PredecessorFMap, typename PredecessorBMap, typename DistanceFMap,
		typename DistanceBMap, typename WeightMap, typename IndexMap, typename ColorFMap, typename ColorBMap,
		typename VertexOrderMap, typename DirectionMap>
		OldHLOrderStrategy<Graph, PredecessorFMap, PredecessorBMap, DistanceFMap,
		DistanceBMap, WeightMap, IndexMap, ColorFMap, ColorBMap,
		VertexOrderMap, DirectionMap> GenerateOldHLOrderStrategy(Graph& graph,
			PredecessorFMap& predecessorF, PredecessorBMap& predecessorB,
			DistanceFMap& distanceF, DistanceBMap& distanceB,
			WeightMap& weight, IndexMap& index,
			ColorFMap& colorF, ColorBMap& colorB,
			DirectionMap& direction, VertexOrderMap& oreder, size_t dijLimit) {
		return OldHLOrderStrategy<Graph, PredecessorFMap, PredecessorBMap, DistanceFMap,
			DistanceBMap, WeightMap, IndexMap, ColorFMap, ColorBMap,
			VertexOrderMap, DirectionMap>(graph, predecessorF, predecessorB, distanceF,
				distanceB, weight, index, colorF, colorB, direction, oreder, dijLimit);
	}


	template <typename Graph>
	class VertexDegreeOrderStrategy {
		using Vertex = typename graph::graph_traits<Graph>::vertex_descriptor;
		using VertexOrder = std::vector<Vertex>;
		using VertexIterator = typename VertexOrder::iterator;
		VertexOrder order;
		VertexIterator current;

	public:

		VertexDegreeOrderStrategy(Graph& graph) {
			order = graphUtil::AsArray(graphUtil::Range(vertices(graph)));
			std::mt19937 generator(3561237589);
			std::shuffle(order.begin(), order.end(), generator); // ch is 15% faster
			std::stable_sort(order.begin(), order.end(),
			                 [&graph](const Vertex& a, const Vertex& b) -> bool {
				                 auto a_degree = degree(a, graph);
				                 auto b_degree = degree(b, graph);
				                 return a_degree < b_degree;
			                 });
			current = order.begin();
			std::cout << "Initialization done" << std::endl;
		}

		typename graph::graph_traits<Graph>::vertex_descriptor
		next(Graph& graph) {
			if (current != order.end()) {
				auto v = *current;
				++current;
				return v;
			}
			return graph.null_vertex();
		};
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
			if (current != order.end()) {
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
}


#pragma once

#include <tuple>
#include <vector>
#include <random>
#include <graph/graph.hpp>
#include <graph/detail/util/Collection.hpp>

namespace ch
{
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

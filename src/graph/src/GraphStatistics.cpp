#include "graph\GraphStatistics.hpp"
#include <algorithm>

namespace GraphStatistics {
	graph::StaticGraph::degree_size_type GetMaximalVertexDegree(const graph::StaticGraph& graph) {
		graph::StaticGraph::degree_size_type maxDegree = 0;
		auto vertexIterators = vertices(graph);
		for (auto it = vertexIterators.first; it != vertexIterators.second; ++it) {
			maxDegree = std::max(maxDegree, out_degree(*it, graph));
		}

		return maxDegree;
	}

	int GetMinimalVertexDegree(const graph::StaticGraph& graph) {
		graph::StaticGraph::degree_size_type minDegree = 1;
		auto vertexIterators = vertices(graph);
		for (auto it = vertexIterators.first; it != vertexIterators.second; ++it) {
			auto vertexDegree = out_degree(*it, graph);
			if (vertexDegree > 0)
				minDegree = std::min(minDegree, vertexDegree);
		}

		return minDegree;
	}

	double GetAverageVertexDegree(const graph::StaticGraph& graph) {
		int nonEmptyVertexCount = 0;
		auto vertexIterators = vertices(graph);
		for (auto it = vertexIterators.first; it != vertexIterators.second; ++it) {
			auto vertexDegree = out_degree(*it, graph);
			if (vertexDegree > 0)
				nonEmptyVertexCount++;
		}

		return (double)num_edges(graph) / (double)nonEmptyVertexCount;
	}

	int GetSingleOrientedEdgesCount(const graph::StaticGraph& graph) {
		int result = 0;

		auto vertexIterators = vertices(graph);
		for (auto vIt = vertexIterators.first; vIt != vertexIterators.second; ++vIt) {
			auto adjacencyIterators = graph::adjacent_vertices(*vIt, graph);
			for (auto edgeIt = adjacencyIterators.first; edgeIt != adjacencyIterators.second; ++edgeIt) {
				if (!edge(*edgeIt, *vIt, graph).second)
					++result;
			}
		}

		return result;
	}
}

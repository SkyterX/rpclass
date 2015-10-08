#include "graph\GraphStatistics.hpp"
#include <algorithm>

namespace GraphStatistics
{
	graph::StaticGraph::degree_size_type GetMaximalVertexDegree(const graph::StaticGraph& graph) {
		graph::StaticGraph::degree_size_type maxDegree = 0;

		for (auto& v : graph.Vertices) {
			maxDegree = std::max(maxDegree, out_degree(v, graph));
		}

		return maxDegree;
	}

	int GetMinimalVertexDegree(const graph::StaticGraph& graph) {
		graph::StaticGraph::degree_size_type minDegree = 1;

		for (auto &v : graph.Vertices) {
			auto vertexDegree = out_degree(v, graph);
			if (vertexDegree > 0)
				minDegree = std::min(minDegree, vertexDegree);
		}

		return minDegree;
	}

	double GetAverageVertexDegree(const graph::StaticGraph& graph) {
		int nonEmptyVertexCount = 0;

		for (auto& v : graph.Vertices) {
			auto vertexDegree = out_degree(v, graph);
			if (vertexDegree > 0)
				nonEmptyVertexCount++;
		}

		return (double)num_edges(graph) / (double)nonEmptyVertexCount;
	}

	int GetSingleOrientedEdgesCount(const graph::StaticGraph& graph) {
		int result = 0;

		for (auto &source : graph.Vertices) {
			for (auto &target : graph.Edges(source)) {
				if (!edge(target, source, graph).second)
					++result;
			}
		}

		return result;
	}
}

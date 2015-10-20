#pragma once

#include <graph/static_graph.hpp>
#include <cassert>
#include <algorithm>

namespace GraphStatistics {
	inline graph::StaticGraph::degree_size_type GetMaximalVertexDegree(const graph::StaticGraph& graph) {
		graph::StaticGraph::degree_size_type maxDegree = 0;

		for (auto& v : graph.Vertices()) {
			maxDegree = std::max(maxDegree, out_degree(v, graph));
		}

		return maxDegree;
	}

	inline graph::StaticGraph::degree_size_type GetMinimalVertexDegree(const graph::StaticGraph& graph) {
		graph::StaticGraph::degree_size_type minDegree = 1;

		for (auto& v : graph.Vertices()) {
			auto vertexDegree = out_degree(v, graph);
			if (vertexDegree > 0)
				minDegree = std::min(minDegree, vertexDegree);
		}

		return minDegree;
	}

	inline double GetAverageVertexDegree(const graph::StaticGraph& graph) {
		graph::StaticGraph::vertices_size_type nonEmptyVertexCount = 0;

		for (auto& v : graph.Vertices()) {
			auto vertexDegree = out_degree(v, graph);
			if (vertexDegree > 0)
				nonEmptyVertexCount++;
		}

		return (double)num_edges(graph) / (double)nonEmptyVertexCount;
	}

	inline graph::StaticGraph::edges_size_type GetSingleOrientedEdgesCount(const graph::StaticGraph& graph) {
		graph::StaticGraph::edges_size_type result = 0;

		for (auto& source : graph.Vertices()) {
			for (auto& target : graph.OutAdjacencies(source)) {
				if (!graph.InAdjacencies(source).contains(target))
					++result;
			}
		}

		return result;
	}
}

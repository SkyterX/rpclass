#pragma once

#include <graph/static_graph.hpp>
#include <cassert>
#include <limits>
#include <algorithm>

namespace GraphStatistics {
#define Properties VertexProperties, EdgeProperties

	template <typename VertexProperties, typename EdgeProperties>
	inline typename graph::StaticGraph<Properties>::degree_size_type GetMaximalVertexDegree(
		const graph::StaticGraph<Properties>& graph) {
		using DegreeType = typename graph::StaticGraph<Properties>::degree_size_type;
		DegreeType maxDegree = std::numeric_limits<DegreeType>::min();

		for (auto& v : graph.Vertices()) {
			maxDegree = std::max(maxDegree, out_degree(v, graph));
		}

		return maxDegree;
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename graph::StaticGraph<Properties>::degree_size_type GetMinimalVertexDegree(
		const graph::StaticGraph<Properties>& graph) {
		using DegreeType = typename graph::StaticGraph<Properties>::degree_size_type;
		DegreeType minDegree = std::numeric_limits<DegreeType>::max();

		for (auto& v : graph.Vertices()) {
			auto vertexDegree = out_degree(v, graph);
			if (vertexDegree > 0)
				minDegree = std::min(minDegree, vertexDegree);
		}

		return minDegree;
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline double GetAverageVertexDegree(const graph::StaticGraph<Properties>& graph) {
		using VerticesSizeType = typename graph::StaticGraph<Properties>::vertices_size_type;
		VerticesSizeType nonEmptyVertexCount = 0;

		for (auto& v : graph.Vertices()) {
			auto vertexDegree = out_degree(v, graph);
			if (vertexDegree > 0)
				++nonEmptyVertexCount;
		}

		return (double)num_edges(graph) / (double)nonEmptyVertexCount;
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename graph::StaticGraph<Properties>::edges_size_type GetSingleOrientedEdgesCount(
		const graph::StaticGraph<Properties>& graph) {
		using EdgesSizeType = typename graph::StaticGraph<Properties>::vertices_size_type;
		EdgesSizeType result = 0;

		for (auto& source : graph.Vertices()) {
			for (auto& target : graph.OutAdjacencies(source)) {
				if (!graph.InAdjacencies(source).contains(target))
					++result;
			}
		}

		return result;
	}
}

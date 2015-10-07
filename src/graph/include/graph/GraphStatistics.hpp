#pragma once

#include "graph/static_graph.hpp"

namespace GraphStatistics {
	graph::StaticGraph::degree_size_type GetMaximalVertexDegree(const graph::StaticGraph& graph);
	int GetMinimalVertexDegree(const graph::StaticGraph& graph);
	double GetAverageVertexDegree(const graph::StaticGraph& graph);
	int GetSingleOrientedEdgesCount(const graph::StaticGraph& graph);
}

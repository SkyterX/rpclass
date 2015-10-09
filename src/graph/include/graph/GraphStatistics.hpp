#pragma once

#include "graph/static_graph.hpp"

namespace GraphStatistics {
	graph::StaticGraph::degree_size_type GetMaximalVertexDegree(const graph::StaticGraph& graph);
	graph::StaticGraph::degree_size_type GetMinimalVertexDegree(const graph::StaticGraph& graph);
	double GetAverageVertexDegree(const graph::StaticGraph& graph);
	graph::StaticGraph::edges_size_type GetSingleOrientedEdgesCount(const graph::StaticGraph& graph);
}

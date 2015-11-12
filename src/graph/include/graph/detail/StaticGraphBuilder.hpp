#pragma once
#include <algorithm>

namespace graph {
	template <typename Graph>
	class GraphBuilder {
		friend Graph;
		using vertices_size_type = typename Graph::vertices_size_type;
		using edges_size_type = typename Graph::edges_size_type;
		using vertex_descriptor = typename Graph::vertex_descriptor;
		using degree_size_type = typename Graph::degree_size_type;

		using EdgePropertiesType = typename Graph::edge_bundled;
		using EdgeType = FancyEdge<vertex_descriptor, EdgePropertiesType>;
		using VertexType = typename Graph::VertexType;
		using StoredAdjacencyType = typename Graph::StoredAdjacencyType;
		using AdjacenciesVecType = typename Graph::AdjacenciesVecType;

	public:
		GraphBuilder(vertices_size_type vertexCount, edges_size_type edgesCount = 0) {
			this->unsortedEdges.reserve(edgesCount);
			this->vertexCount = vertexCount;
		}

		void AddEdge(const vertex_descriptor& from, const vertex_descriptor& to,
					 const EdgePropertiesType& properties = EdgePropertiesType()) {
			unsortedEdges.push_back(EdgeType(from, to, properties));
		}

		std::unique_ptr<Graph> Build() {
			auto graph = std::make_unique<Graph>();
			BuildGraph(*graph);
			return graph;
		}

	private:
		void BuildGraph(Graph& graph) {
			SortEdgesAndCopyTo(graph);
			graph.Initialize();
		}

		void SortEdgesAndCopyTo(Graph& graph) {

			graph.vertices.reserve(this->vertexCount);
			graph.edgesSeparators.reserve(this->vertexCount);
			graph.adjacencies.resize(2 * this->unsortedEdges.size());
			graph.edgeProperties.reserve(this->unsortedEdges.size());

			auto inDegreeCounts = std::vector<degree_size_type>(this->vertexCount, 0);
			auto outDegreeCounts = std::vector<degree_size_type>(this->vertexCount, 0);
			auto linkPointers = std::vector<typename AdjacenciesVecType::iterator>();
			linkPointers.reserve(this->vertexCount + 1);

			for (auto& edge : this->unsortedEdges) {
				++outDegreeCounts[edge.source];
				++inDegreeCounts[edge.target];
			}

			linkPointers.push_back(graph.adjacencies.begin());
			graph.vertices.push_back(VertexType(linkPointers[0]));
			graph.edgesSeparators.push_back(inDegreeCounts[0]);
			outDegreeCounts[0] += inDegreeCounts[0];
			for (int i = 1; i < this->vertexCount; ++i) {
				linkPointers.push_back(linkPointers[i - 1] + outDegreeCounts[i - 1]);
				graph.vertices.push_back(VertexType(linkPointers[i]));
				graph.edgesSeparators.push_back(inDegreeCounts[i]);
				outDegreeCounts[i] += inDegreeCounts[i];
			}
			linkPointers.push_back(graph.adjacencies.end());

			for (auto& edge : this->unsortedEdges) {
				auto from = edge.source;
				auto to = edge.target;
				graph.edgeProperties.push_back(edge.properties);
				auto propertyPtr = &graph.edgeProperties.back();
				--outDegreeCounts[from];
				*(linkPointers[from] + outDegreeCounts[from]) = StoredAdjacencyType(to, propertyPtr);
				--inDegreeCounts[to];
				*(linkPointers[to] + inDegreeCounts[to]) = StoredAdjacencyType(from, propertyPtr);
			}

			for (int i = 0; i < this->vertexCount; ++i) {
				auto start = linkPointers[i];
				auto end = linkPointers[i + 1];
				std::stable_sort(start, start + graph.edgesSeparators[i],
						  [](const StoredAdjacencyType& a, const StoredAdjacencyType& b) {
							  return a.target < b.target;
						  });
				std::stable_sort(start + graph.edgesSeparators[i], end,
						  [](const StoredAdjacencyType& a, const StoredAdjacencyType& b) {
							  return a.target < b.target;
						  });
				start = end;
			}
		}

		std::vector<EdgeType> unsortedEdges;
		int vertexCount;
	};
}

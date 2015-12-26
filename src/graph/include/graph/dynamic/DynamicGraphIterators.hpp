#pragma once
#include <type_traits>
#include <limits>
#include <boost/iterator/iterator_facade.hpp>

namespace graph {

	template <typename Graph>
	class DGAdjacencyIterator
			: public boost::iterator_facade<
				DGAdjacencyIterator<Graph>,
				const typename Graph::vertex_descriptor,
				boost::random_access_traversal_tag> {
		using BaseType = boost::iterator_facade<
			DGAdjacencyIterator<Graph>,
			const typename Graph::vertex_descriptor,
			boost::random_access_traversal_tag>;
		using DifferenceType = typename BaseType::difference_type;
		using VertexDescriptor = typename Graph::vertex_descriptor;
		const size_t nullLink = std::numeric_limits<size_t>::max();
	public:
		DGAdjacencyIterator(DifferenceType size)
			: index(size), linkIndex(nullLink), graph(nullptr) {}

		DGAdjacencyIterator(const Graph* graph, const VertexDescriptor& vertex)
			: index(0), graph(graph) {
			linkIndex = graph->vertices[vertex].firstEdgeIndex;
		}

	private:
		friend class boost::iterator_core_access;

		DifferenceType distance_to(DGAdjacencyIterator<Graph> const& other) const {
			return other.index - this->index;
		}

		bool equal(DGAdjacencyIterator<Graph> const& other) const {
			return this->linkIndex == other.linkIndex;
		}

		void increment() {
			++index;
			linkIndex = graph->adjacencies[linkIndex].nextLink;
		}

		const VertexDescriptor& dereference() const {
			return graph->adjacencies[linkIndex].target;
		}

		DifferenceType index;
		size_t linkIndex;
		const Graph* graph;
	};

	template <typename Graph>
	class DGEdgeIterator
			: public boost::iterator_facade<
				DGEdgeIterator<Graph>,
				typename Graph::edge_descriptor,
				boost::random_access_traversal_tag,
				typename Graph::edge_descriptor> {
		using BaseType = boost::iterator_facade<
			DGEdgeIterator<Graph>,
			const typename Graph::edge_descriptor,
			boost::random_access_traversal_tag>;
		using DifferenceType = typename BaseType::difference_type;
		using VertexDescriptor = typename Graph::vertex_descriptor;
		using EdgeDescriptor = typename Graph::edge_descriptor;
		using EdgeProperties = typename Graph::edge_bundled;
		const size_t nullLink = std::numeric_limits<size_t>::max();
	public:
		DGEdgeIterator(DifferenceType size)
			: index(size), linkIndex(nullLink), graph(nullptr) {}

		DGEdgeIterator(Graph* graph, const VertexDescriptor& vertex)
			: index(0), vertex(vertex), graph(graph) {
			linkIndex = graph->vertices[vertex].firstEdgeIndex;
		}

	private:
		friend class boost::iterator_core_access;

		DifferenceType distance_to(const DGEdgeIterator<Graph>& other) const {
			return other.index - this->index;
		}

		bool equal(const DGEdgeIterator<Graph>& other) const {
			return this->linkIndex == other.linkIndex;
		}

		void increment() {
			++index;
			linkIndex = graph->adjacencies[linkIndex].nextLink;
		}

		EdgeDescriptor dereference() const {
			auto& link = graph->adjacencies[linkIndex];
			VertexDescriptor from = vertex;
			VertexDescriptor to = link.target;
			EdgeProperties* props = &graph->adjacencies[linkIndex].properties;
			return EdgeDescriptor(from, to, props);
		}

		DifferenceType index;
		VertexDescriptor vertex;
		size_t linkIndex;
		Graph* graph;
	};
}

#pragma once
#include <vector>
#include <cinttypes>
#include <boost/graph/graph_traits.hpp>
#include <memory>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/property_map/property_map.hpp>
#include "util/Collection.hpp"
#include "StaticGraphIterators.hpp"
#include "BasicGraphStructures.hpp"
#include "StaticGraphPropertyMaps.hpp"
#include "properties.hpp"

namespace graph {
	struct StaticGraphTraversalCategory :
			public boost::bidirectional_graph_tag,
			public boost::vertex_list_graph_tag { };

	struct vertex_properties_stub {};
	struct edge_properties_stub {};


	class StaticGraph {
		class VertexIterator;
		using VertexProperties = vertex_properties_stub;
		using EdgeProperties = edge_properties_stub;
	public:
		using type = StaticGraph;

		using edge_size_type = uint32_t;
		using vertices_size_type = uint32_t;
		using edges_size_type = uint32_t;
		using degree_size_type = uint16_t;
		using vertex_descriptor = vertices_size_type;
		using directed_category = boost::directed_tag;
		using edge_parallel_category = boost::disallow_parallel_edge_tag;
		using traversal_category = StaticGraphTraversalCategory;

		using EdgeType = FancyEdge<vertex_descriptor, EdgeProperties>;
		using edge_descriptor = FancyEdgeDescriptor<EdgeType::VertexType, EdgeType::EdgePropertiesType>;

	private:
		using AdjacenciesVecType = std::vector<EdgeType*>;
		using AdjacenciesVecIteratorType = AdjacenciesVecType::const_iterator;

	public:
		using edge_iterator = EdgeIterator<AdjacenciesVecIteratorType, edge_descriptor>;
		using adjacency_iterator = AdjacencyIterator<AdjacenciesVecIteratorType, vertex_descriptor>;
		using out_edge_iterator = edge_iterator;
		using in_edge_iterator = edge_iterator;
		using VertexType = Vertex<AdjacenciesVecIteratorType, VertexProperties>;

	private:
		using VerticesVecType = std::vector<VertexType>;
		using EdgesVecType = std::vector<EdgeType>;
		using AdjacenciesSeparatorsVecType = std::vector<degree_size_type>;
	public:

		using vertex_iterator = boost::counting_iterator<vertex_descriptor>;
		class Builder;
		class VertexCollection;
		class AdjacencyCollection;
		class EdgeCollection;
		using VertexPropertyMapType = VertexPropertyMap<VertexProperties, type>;
		using EdgePropertyMapType = EdgePropertyMap<EdgeProperties, type>;
		friend class VertexPropertyMapType;

		

		StaticGraph();

		template <class PairIterator>
		StaticGraph(PairIterator begin, PairIterator end,
					vertices_size_type n, edges_size_type m = 0);

		const VertexCollection& Vertices() const;
		AdjacencyCollection OutAdjacencies(const vertex_descriptor& v) const;
		AdjacencyCollection InAdjacencies(const vertex_descriptor& v) const;
		EdgeCollection OutEdges(const vertex_descriptor& v) const;
		EdgeCollection InEdges(const vertex_descriptor& v) const;
		edges_size_type EdgesCount() const;

		const EdgePropertyMapType& GetEdgePropertyMap() const;
		const VertexPropertyMapType& GetVertexPropertyMap() const;
	private:
		void Initialize();

		AdjacenciesVecType adjacencies;
		VerticesVecType vertices;
		EdgesVecType edges;
		AdjacenciesSeparatorsVecType edgesSeparators;
		std::unique_ptr<EdgePropertyMapType> edgePropertyMap;
		std::unique_ptr<VertexPropertyMapType> vertexPropertyMap;
		std::unique_ptr<VertexCollection> vertexCollection;
	};

	class StaticGraph::VertexCollection : public graphUtil::Collection<vertex_iterator> {
	public:
		VertexType operator[](const vertex_descriptor& v) const {
			if (v == this->size())
				return VertexType(this->graph.adjacencies.end());
			return this->graph.vertices[v];
		}

	private:
		friend class StaticGraph;

		explicit VertexCollection(const StaticGraph& g)
			: CollectionType(vertex_iterator(0), vertex_iterator(g.vertices.size())), graph(g) {}

		const StaticGraph& graph;
	};

	class StaticGraph::AdjacencyCollection :
			public graphUtil::ValueCollection<adjacency_iterator, adjacency_iterator::value_type, true> {
	private:
		friend class StaticGraph;

		AdjacencyCollection(const adjacency_iterator& first, const adjacency_iterator& last)
			: CollectionType(first, last) {}
	};

	class StaticGraph::EdgeCollection : public graphUtil::Collection<edge_iterator> {
	private:
		friend class StaticGraph;

		EdgeCollection(const edge_iterator& first, const edge_iterator& last)
			: CollectionType(first, last) {}
	};

	class StaticGraph::Builder {
		friend class StaticGraph;

	public:
		Builder(vertices_size_type vertexCount, edges_size_type edgesCount = 0);
		virtual ~Builder();
		void AddEdge(const vertex_descriptor& from, const vertex_descriptor& to);
		std::unique_ptr<StaticGraph> Build();
	private:
		void BuildGraph(StaticGraph& graph);
		void SortEdgesAndCopyTo(StaticGraph& graph);

		std::vector<EdgeType> unsortedEdges;
		int vertexCount;
	};
}

// PropertyMaps
namespace graph {

	template <>
	struct property_map<StaticGraph, vertex_bundle_t> {
		using type = StaticGraph::VertexPropertyMapType;
	};

	template <>
	struct property_map<StaticGraph, edge_bundle_t> {
		using type = StaticGraph::EdgePropertyMapType;
	};

	template <>
	inline property_map<StaticGraph, vertex_bundle_t>::type
	get<StaticGraph>(const vertex_bundle_t&, StaticGraph& graph) {
		return graph.GetVertexPropertyMap();
	}

	template <>
	inline property_map<StaticGraph, edge_bundle_t>::type
	get<StaticGraph>(const edge_bundle_t&, StaticGraph& graph) {
		return graph.GetEdgePropertyMap();
	}
}

#include "StaticGraphTools.hpp"
#include "StaticGraphBuilder.hpp"

// StaticGraph
namespace graph {
	inline StaticGraph::StaticGraph()
		: edgePropertyMap(),
		  vertexPropertyMap(new VertexPropertyMapType(*this)),
		  vertexCollection(nullptr) {}

	template <class PairIterator>
	StaticGraph::StaticGraph(PairIterator begin, PairIterator end, vertices_size_type n, edges_size_type m) : StaticGraph() {
		auto builder = new Builder(n, m);

		for (auto& it = begin; it != end; ++it) {
			builder->AddEdge(it->first, it->second);
		}

		builder->BuildGraph(*this);

		delete builder;
	}

	inline const StaticGraph::VertexCollection& StaticGraph::Vertices() const {
		return *vertexCollection;
	}

	inline StaticGraph::AdjacencyCollection StaticGraph::InAdjacencies(const vertex_descriptor& v) const {
		return AdjacencyCollection(
			adjacency_iterator(this->Vertices()[v].begin, true),
			adjacency_iterator(this->Vertices()[v].begin + this->edgesSeparators[v], true));
	}

	inline StaticGraph::AdjacencyCollection StaticGraph::OutAdjacencies(const vertex_descriptor& v) const {
		return AdjacencyCollection(
			adjacency_iterator(this->Vertices()[v].begin + this->edgesSeparators[v], false),
			adjacency_iterator(this->Vertices()[v + 1].begin, false));
	}

	inline StaticGraph::EdgeCollection StaticGraph::InEdges(const vertex_descriptor& v) const {
		return EdgeCollection(
			edge_iterator(this->Vertices()[v].begin),
			edge_iterator(this->Vertices()[v].begin + this->edgesSeparators[v]));
	}

	inline StaticGraph::EdgeCollection StaticGraph::OutEdges(const vertex_descriptor& v) const {
		return EdgeCollection(
			edge_iterator(this->Vertices()[v].begin + this->edgesSeparators[v]),
			edge_iterator(this->Vertices()[v + 1].begin));
	}

	inline void StaticGraph::Initialize() {
		this->vertexCollection = std::unique_ptr<VertexCollection>(new VertexCollection(*this));
	}

	inline const StaticGraph::VertexPropertyMapType& StaticGraph::GetVertexPropertyMap() const {
		return *vertexPropertyMap;
	}

	inline const StaticGraph::EdgePropertyMapType& StaticGraph::GetEdgePropertyMap() const {
		return *edgePropertyMap;
	}

	inline StaticGraph::edges_size_type StaticGraph::EdgesCount() const {
		return edges.size();
	}
}

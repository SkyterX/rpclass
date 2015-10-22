#pragma once
#include <vector>
#include <cinttypes>
#include <boost/graph/graph_traits.hpp>
#include <memory>
#include <boost/iterator/counting_iterator.hpp>
#include "util/Collection.hpp"
#include "StaticGraphIterators.hpp"
#include "BasicGraphStructures.hpp"
#include "StaticGraphPropertyMaps.hpp"
#include "StaticGraphBuilder.hpp"
#include "properties.hpp"

namespace graph {

	struct StaticGraphTraversalCategory :
		public adjacency_graph_tag, public boost::adjacency_graph_tag,
		public bidirectional_graph_tag, public boost::bidirectional_graph_tag,
		public vertex_list_graph_tag, public boost::vertex_list_graph_tag
	{ };

	struct NoProperties : public Properties<> {};

	template <typename VertexProperties = NoProperties, typename EdgeProperties = NoProperties>
	class StaticGraph {
	public:
		using type = StaticGraph<VertexProperties, EdgeProperties>;

		using edge_size_type = uint32_t;
		using vertices_size_type = uint32_t;
		using edges_size_type = uint32_t;
		using degree_size_type = uint16_t;
		using vertex_descriptor = vertices_size_type;
		using directed_category = directed_tag;
		using edge_parallel_category = disallow_parallel_edge_tag;
		using traversal_category = StaticGraphTraversalCategory;

		using VertexPropertyMapType = VertexPropertyMap<VertexProperties, type>;
		using EdgePropertyMapType = EdgePropertyMap<EdgeProperties, type>;
		using vertex_bundled = VertexProperties;
		using edge_bundled = EdgeProperties;

	private:
		using EdgeType = FancyEdge<vertex_descriptor, EdgeProperties>;
		using EdgesVecType = std::vector<EdgeType>;

		using AdjacenciesVecType = std::vector<EdgeType*>;
		using AdjacenciesVecIteratorType = typename AdjacenciesVecType::const_iterator;

		using VertexType = Vertex<AdjacenciesVecIteratorType, VertexProperties>;
		using VerticesVecType = std::vector<VertexType>;

		using AdjacenciesSeparatorsVecType = std::vector<degree_size_type>;
	public:
		using vertex_iterator = boost::counting_iterator<vertex_descriptor>;
		using in_adjacency_iterator = AdjacencyIterator<AdjacenciesVecIteratorType, vertex_descriptor, internals::EdgeDirection::In>;
		using out_adjacency_iterator = AdjacencyIterator<AdjacenciesVecIteratorType, vertex_descriptor, internals::EdgeDirection::Out>;
		using adjacency_iterator = out_adjacency_iterator;
		using edge_descriptor = FancyEdgeDescriptor<typename EdgeType::VertexType, typename EdgeType::EdgePropertiesType>;
		using edge_iterator = EdgeIterator<AdjacenciesVecIteratorType, edge_descriptor>;
		using out_edge_iterator = edge_iterator;
		using in_edge_iterator = edge_iterator;


		using EdgeCollection = graphUtil::Collection<edge_iterator>;
		using InAdjacencyCollection = graphUtil::ValueCollection<in_adjacency_iterator, graphUtil::SortedTag>;
		using OutAdjacencyCollection = graphUtil::ValueCollection<out_adjacency_iterator, graphUtil::SortedTag>;

		using Builder = GraphBuilder<type>;
		friend Builder;
		
		class VertexCollection : public graphUtil::Collection<vertex_iterator> {
		public:
			VertexType operator[](const vertex_descriptor& v) const {
				if (v == this->size())
					return VertexType(this->graph.adjacencies.end());
				return this->graph.vertices[v];
			}

			explicit VertexCollection(const StaticGraph& g)
				: CollectionType(vertex_iterator(0), vertex_iterator(g.vertices.size())), graph(g) {}

		private:
			const StaticGraph& graph;
		};

		friend class VertexPropertyMapType;


		StaticGraph()
			: edgePropertyMap(new EdgePropertyMapType()),
			  vertexPropertyMap(new VertexPropertyMapType(*this)),
			  vertexCollection(nullptr) {}

		template <class PairIterator>
		StaticGraph(PairIterator begin, PairIterator end,
					vertices_size_type n, edges_size_type m = 0) : StaticGraph() {
			auto builder = new Builder(n, m);

			for (auto& it = begin; it != end; ++it) {
				builder->AddEdge(it->first, it->second);
			}

			builder->BuildGraph(*this);

			delete builder;
		}

		const VertexCollection& Vertices() const {
			return *vertexCollection;
		}

		OutAdjacencyCollection OutAdjacencies(const vertex_descriptor& v) const {
			return OutAdjacencyCollection(
				out_adjacency_iterator(this->Vertices()[v].begin + this->edgesSeparators[v]),
				out_adjacency_iterator(this->Vertices()[v + 1].begin));
				
		}

		InAdjacencyCollection InAdjacencies(const vertex_descriptor& v) const {
			return InAdjacencyCollection(
				in_adjacency_iterator(this->Vertices()[v].begin),
				in_adjacency_iterator(this->Vertices()[v].begin + this->edgesSeparators[v]));
		}

		EdgeCollection OutEdges(const vertex_descriptor& v) const {
			return EdgeCollection(
				edge_iterator(this->Vertices()[v].begin + this->edgesSeparators[v]),
				edge_iterator(this->Vertices()[v + 1].begin));
		}

		EdgeCollection InEdges(const vertex_descriptor& v) const {
			return EdgeCollection(
				edge_iterator(this->Vertices()[v].begin),
				edge_iterator(this->Vertices()[v].begin + this->edgesSeparators[v]));
		}

		edges_size_type EdgesCount() const {
			return edges.size();
		}

		const EdgePropertyMapType& GetEdgePropertyMap() const {
			return *edgePropertyMap;
		}

		const VertexPropertyMapType& GetVertexPropertyMap() const {
			return *vertexPropertyMap;
		}

	private:
		void Initialize() {
			this->vertexCollection = std::make_unique<VertexCollection>(*this);
		}

		AdjacenciesVecType adjacencies;
		VerticesVecType vertices;
		EdgesVecType edges;
		AdjacenciesSeparatorsVecType edgesSeparators;
		std::unique_ptr<EdgePropertyMapType> edgePropertyMap;
		std::unique_ptr<VertexPropertyMapType> vertexPropertyMap;
		std::unique_ptr<VertexCollection> vertexCollection;
	};
}

// graph_traits
namespace graph {
	template<typename VertexProperties, typename EdgeProperties>
	struct graph_traits<StaticGraph<VertexProperties, EdgeProperties>> 	: 
		public virtual graph_traits<StaticGraph<VertexProperties, EdgeProperties>, adjacency_graph_tag>,
		public virtual graph_traits<StaticGraph<VertexProperties, EdgeProperties>, bidirectional_graph_tag>,
		public virtual graph_traits<StaticGraph<VertexProperties, EdgeProperties>, vertex_list_graph_tag>	
	{};
}

// PropertyMaps
namespace graph {
#define StaticGraphType StaticGraph<VertexProperties, EdgeProperties>

	template <typename VertexProperties, typename EdgeProperties>
	struct property_map<StaticGraphType, vertex_bundle_t> {
		using type = typename StaticGraphType::VertexPropertyMapType;
	};

	template <typename VertexProperties, typename EdgeProperties>
	struct property_map<StaticGraphType, edge_bundle_t> {
		using type = typename StaticGraphType::EdgePropertyMapType;
	};

	template <typename VertexProperties, typename EdgeProperties>
	inline typename property_map<StaticGraphType, vertex_bundle_t>::type
	get(const vertex_bundle_t&, StaticGraphType& graph) {
		return graph.GetVertexPropertyMap();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename property_map<StaticGraphType, edge_bundle_t>::type
	get(const edge_bundle_t&, StaticGraphType& graph) {
		return graph.GetEdgePropertyMap();
	}
#undef StaticGraphType
}

#include "StaticGraphTools.hpp"

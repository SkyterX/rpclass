#pragma once
#include <vector>
#include <cinttypes>
#include <boost/graph/graph_traits.hpp>
#include <memory>
#include <limits>
#include <boost/iterator/counting_iterator.hpp>
#include <graph/detail/util/Collection.hpp>
#include <graph/properties.hpp>
#include <graph/dynamic/DynamicGraphStructure.hpp>
#include <graph/dynamic/DynamicGraphIterators.hpp>
#include <graph/detail/StaticGraphPropertyMaps.hpp>

namespace graph {

	struct DGTraversalCategory :
		public adjacency_graph_tag, public boost::adjacency_graph_tag,
		public incidence_graph_tag, public boost::incidence_graph_tag,
		public vertex_list_graph_tag, public boost::vertex_list_graph_tag
	{ };

	struct DGNoProperties : public Properties<> {};

	template <typename VertexProperties = DGNoProperties, typename EdgeProperties = DGNoProperties>
	class DynamicGraph {
//		using VertexProperties = DGNoProperties;
//		using EdgeProperties = DGNoProperties;
//		using type = DynamicGraph;
		using type = DynamicGraph<VertexProperties, EdgeProperties>;
	public:

		using edge_size_type = uint32_t;
		using vertices_size_type = uint32_t;
		using edges_size_type = uint32_t;
		using degree_size_type = uint16_t;
		using vertex_descriptor = vertices_size_type;
		using directed_category = directed_tag;
		using edge_parallel_category = disallow_parallel_edge_tag;
		using traversal_category = DGTraversalCategory;

		using VertexPropertyMapType = VertexPropertyMap<VertexProperties, type>;
		using EdgePropertyMapType = EdgePropertyMap<EdgeProperties, type>;
		using vertex_bundled = VertexProperties;
		using edge_bundled = EdgeProperties;

		friend VertexPropertyMapType;
	private:

		using EdgePropertiesVecType = std::vector<EdgeProperties>;
		
		const size_t nullLink = std::numeric_limits<size_t>::max();
		using StoredAdjacencyType = DGFancyLink<vertex_descriptor, EdgeProperties>;
		using AdjacenciesVecType = std::vector<StoredAdjacencyType>;
		using AdjacenciesVecIteratorType = typename AdjacenciesVecType::const_iterator;

		using VertexType = DGVertex<type>;
		using VerticesVecType = std::vector<VertexType>;

		using AdjacenciesSeparatorsVecType = std::vector<degree_size_type>;
	public:
		using vertex_iterator = boost::counting_iterator<vertex_descriptor>;
		using adjacency_iterator = DGAdjacencyIterator<type>;
		using edge_descriptor = FancyEdgeDescriptor<typename StoredAdjacencyType::VertexType, typename StoredAdjacencyType::EdgePropertiesType>;
		using out_edge_iterator = DGEdgeIterator<type>;

		friend adjacency_iterator;
		friend out_edge_iterator;

		using OutEdgeCollection = graphUtil::Collection<out_edge_iterator>;
		using AdjacencyCollection = graphUtil::ValueCollection<adjacency_iterator, graphUtil::SortedTag>;

		using VertexCollection = graphUtil::Collection<vertex_iterator>;

		static vertex_descriptor null_vertex() {
			return std::numeric_limits<vertex_descriptor>::max();
		};

		DynamicGraph(vertices_size_type n = 0, edges_size_type m = 0)
			: edgePropertyMap(new EdgePropertyMapType()),
			vertexPropertyMap(new VertexPropertyMapType(this)) {
			this->vertices.reserve(n);
			this->adjacencies.reserve(m);

			for(auto& v : graphUtil::Range(0, n)) {
				AddVertex();
			}
		}

		template <class PairIterator>
		DynamicGraph(PairIterator begin, PairIterator end,
			vertices_size_type n, edges_size_type m = 0) : DynamicGraph(n, m) {

			for (auto& it = begin; it != end; ++it) {
				EdgeProperties edgeProperties;
				edgeProperties = it->second;
				AddEdge((it->first).first, (it->first).second, edgeProperties);
			}
		}

		DynamicGraph(std::vector<std::pair<size_t, size_t>>::iterator begin,
			std::vector<std::pair<size_t, size_t>>::iterator end,
			size_t n, size_t m = 0) : DynamicGraph(n, m) {

			for (auto& it = begin; it != end; ++it) {
				AddEdge(it->first, it->second);
			}
		}

		vertex_descriptor AddVertex(const VertexProperties& properties = VertexProperties()) {
			vertex_descriptor newVertex = this->vertices.size();
			this->vertices.push_back(VertexType(properties));
			return newVertex;
		}

		std::pair<edge_descriptor, bool> AddEdge(
			const vertex_descriptor& from, const vertex_descriptor& to,
			const EdgeProperties& properties = EdgeProperties()) {

			auto newEdgeIndex = this->adjacencies.size();
			auto& firstEdgeIndex = this->vertices[from].firstEdgeIndex;
			this->adjacencies.push_back(StoredAdjacencyType(to, properties));

			if(firstEdgeIndex == nullLink) {
				firstEdgeIndex = newEdgeIndex;
			}
			else {
				std::swap(this->adjacencies[firstEdgeIndex], this->adjacencies[newEdgeIndex]);
				this->adjacencies[firstEdgeIndex].nextLink = newEdgeIndex;
			}	

			auto propertiesPtr = &this->adjacencies[firstEdgeIndex].properties;
			++this->vertices[from].degree;
			return std::make_pair(edge_descriptor(from, to, propertiesPtr), true);
		}

		degree_size_type Degree(const vertex_descriptor& v) const {
			return this->vertices[v].degree;
		}

		VertexCollection Vertices() const {
			return VertexCollection(vertex_iterator(0), vertex_iterator(this->vertices.size()));
		}

		AdjacencyCollection OutAdjacencies(const vertex_descriptor& v) const {
			return AdjacencyCollection(
				adjacency_iterator(this, v),
				adjacency_iterator(Degree(v)));

		}

		OutEdgeCollection OutEdges(const vertex_descriptor& v) {
			return OutEdgeCollection(
				out_edge_iterator(this, v),
				out_edge_iterator(Degree(v)));
		}

		edges_size_type EdgesCount() const {
			return this->adjacencies.size();
		}

		const EdgePropertyMapType& GetEdgePropertyMap() const {
			return *edgePropertyMap;
		}

		const VertexPropertyMapType& GetVertexPropertyMap() const {
			return *vertexPropertyMap;
		}

	private:

		AdjacenciesVecType adjacencies;
		VerticesVecType vertices;
		std::unique_ptr<EdgePropertyMapType> edgePropertyMap;
		std::unique_ptr<VertexPropertyMapType> vertexPropertyMap;
	};
}

//// PropertyMaps
//namespace graph {
//#define StaticGraphType DynamicGraph
//	//	<VertexProperties, EdgeProperties>
//
//	template <typename VertexProperties, typename EdgeProperties>
//	struct property_map<StaticGraphType, vertex_bundle_t> {
//		using type = typename StaticGraphType::VertexPropertyMapType;
//	};
//
//	template <typename VertexProperties, typename EdgeProperties>
//	struct property_map<StaticGraphType, edge_bundle_t> {
//		using type = typename StaticGraphType::EdgePropertyMapType;
//	};
//
//	template <typename VertexProperties, typename EdgeProperties>
//	inline typename property_map<StaticGraphType, vertex_bundle_t>::type
//		get(const vertex_bundle_t&, StaticGraphType& graph) {
//		return graph.GetVertexPropertyMap();
//	}
//
//	template <typename VertexProperties, typename EdgeProperties>
//	inline typename property_map<StaticGraphType, edge_bundle_t>::type
//		get(const edge_bundle_t&, StaticGraphType& graph) {
//		return graph.GetEdgePropertyMap();
//	}
//
//
//	template <typename Graph>
//	struct VertexIndexPropertyMap {};
//
//	template <typename VertexProperties, typename EdgeProperties>
//	struct VertexIndexPropertyMap<StaticGraphType> {
//		using key_type = typename graph_traits<StaticGraphType>::vertex_descriptor;
//		using value_type = typename graph_traits<StaticGraphType>::vertices_size_type;
//		using reference = value_type&;
//		using category = boost::readable_property_map_tag;
//	};
//
//	template <typename VertexProperties, typename EdgeProperties>
//	struct property_map<StaticGraphType, vertex_index_t> {
//		using type = VertexIndexPropertyMap<StaticGraphType>;
//	};
//
//	template <typename VertexProperties, typename EdgeProperties>
//	typename VertexIndexPropertyMap<StaticGraphType>::value_type
//		get(const VertexIndexPropertyMap<StaticGraphType>& index,
//			const typename VertexIndexPropertyMap<StaticGraphType>::key_type& key) {
//		return key;
//	};
//
//	template<typename VertexProperties, typename EdgeProperties>
//	inline typename property_map<StaticGraphType, vertex_index_t>::type
//		get(const vertex_index_t&, StaticGraphType&) {
//		return VertexIndexPropertyMap<StaticGraphType>();
//	};
//
//#undef StaticGraphType
//}

#include <graph/dynamic/DynamicGraphTools.hpp>
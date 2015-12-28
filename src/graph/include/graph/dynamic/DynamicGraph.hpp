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

namespace graph
{
	struct DGTraversalCategory :
			public adjacency_graph_tag, public boost::adjacency_graph_tag,
			public incidence_graph_tag, public boost::incidence_graph_tag,
			public vertex_list_graph_tag, public boost::vertex_list_graph_tag { };

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

		const edges_size_type nullLink = std::numeric_limits<edges_size_type>::max();
		using StoredAdjacencyType = DGFancyLink<type>;
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
			: freeLinkIndex(nullLink),
			  edgePropertyMap(new EdgePropertyMapType()),
			  vertexPropertyMap(new VertexPropertyMapType(this)) {
			this->vertices.reserve(n);
			this->adjacencies.reserve(m);

			for (auto& v : graphUtil::Range(0, n)) {
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
			OptimizeSpace();
		}

		DynamicGraph(std::vector<std::pair<size_t, size_t>>::iterator begin,
		             std::vector<std::pair<size_t, size_t>>::iterator end,
		             size_t n, size_t m = 0) : DynamicGraph(n, m) {
			for (auto& it = begin; it != end; ++it) {
				AddEdge(it->first, it->second);
			}
			OptimizeSpace();
		}

		vertex_descriptor AddVertex(const VertexProperties& properties = VertexProperties()) {
			vertex_descriptor newVertex = this->vertices.size();
			this->vertices.push_back(VertexType(properties));
			return newVertex;
		}

		std::pair<edge_descriptor, bool> AddEdge(
			const vertex_descriptor& from, const vertex_descriptor& to,
			const EdgeProperties& properties = EdgeProperties()) {

			auto newEdgeIndex = CreateEdge();
			auto firstEdgeIndex = this->vertices[from].firstEdgeIndex;
			this->adjacencies[newEdgeIndex] = StoredAdjacencyType(to, properties);
			if (firstEdgeIndex == nullLink) {
				this->vertices[from].firstEdgeIndex = newEdgeIndex;
			}
			else {
				std::swap(this->adjacencies[firstEdgeIndex], this->adjacencies[newEdgeIndex]);
				this->adjacencies[firstEdgeIndex].nextLink = newEdgeIndex;
				newEdgeIndex = firstEdgeIndex;
			}

			auto propertiesPtr = &this->adjacencies[newEdgeIndex].properties;
			++this->vertices[from].degree;
			return std::make_pair(CreateLinkDescriptor(from, newEdgeIndex), true);
		}

		void RemoveEdge(const vertex_descriptor& from, const vertex_descriptor& to) {
			RemoveEdge(from, [&to] (edge_descriptor& link) {
				           return link.target == to;
			           });
		}

		void RemoveEdge(const edge_descriptor& e) {
			RemoveEdge(e.source, [&e](edge_descriptor& link) {
				           return e == link;
			           });
		}

		template <typename Predicate>
		void RemoveEdge(const vertex_descriptor& v, Predicate predicate) {
			auto edgeIndex = this->vertices[v].firstEdgeIndex;
			while (edgeIndex != nullLink) {
				auto nextEdgeIndex = this->adjacencies[edgeIndex].nextLink;
				auto linkDescriptor = CreateLinkDescriptor(v, edgeIndex);
				if (!predicate(linkDescriptor))
					break;
				DoRemoveEdge(v, edgeIndex);
				this->vertices[v].firstEdgeIndex = nextEdgeIndex;
				edgeIndex = nextEdgeIndex;
			}
			if (edgeIndex == nullLink) return;
			auto nextEdgeIndex = this->adjacencies[edgeIndex].nextLink;
			while (nextEdgeIndex != nullLink) {
				auto linkDescriptor = CreateLinkDescriptor(v, nextEdgeIndex);
				if (predicate(linkDescriptor)) {
					this->adjacencies[edgeIndex].nextLink = this->adjacencies[nextEdgeIndex].nextLink;
					DoRemoveEdge(v, nextEdgeIndex);
				}
				else {
					edgeIndex = nextEdgeIndex;
				}
				nextEdgeIndex = this->adjacencies[edgeIndex].nextLink;
			}
		}

	private:

		edge_descriptor CreateLinkDescriptor(const vertex_descriptor& v, edges_size_type linkIndex) {
			return edge_descriptor(v, this->adjacencies[linkIndex].target, &this->adjacencies[linkIndex].properties);
		}

		void DoRemoveEdge(const vertex_descriptor& v, edges_size_type linkIndex) {
			--this->vertices[v].degree;
			this->adjacencies[linkIndex].nextLink = freeLinkIndex;
			freeLinkIndex = linkIndex;
		}

		edges_size_type CreateEdge() {
			edges_size_type newEdgeIndex;
			if (freeLinkIndex != nullLink) {
				newEdgeIndex = freeLinkIndex;
				freeLinkIndex = this->adjacencies[freeLinkIndex].nextLink;
			}
			else {
				newEdgeIndex = this->adjacencies.size();
				this->adjacencies.push_back(StoredAdjacencyType());
			}
			return newEdgeIndex;
		}

	public:

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

		void OptimizeSpace() {
			std::vector<edges_size_type> permutation;
			permutation.resize(this->adjacencies.size(), nullLink);
			edges_size_type lastFreeId = 0;
			for(auto v : graphUtil::Range(0, this->vertices.size())) {
				auto edgeIndex = this->vertices[v].firstEdgeIndex;
				auto prevEdgeIndex = nullLink;
				while(edgeIndex != nullLink) {
					permutation[edgeIndex] = lastFreeId;
					if (prevEdgeIndex != nullLink)
						this->adjacencies[prevEdgeIndex].nextLink = lastFreeId;
					++lastFreeId;
					prevEdgeIndex = edgeIndex;
					edgeIndex = this->adjacencies[edgeIndex].nextLink;
				}
				if(this->vertices[v].firstEdgeIndex != nullLink)
					this->vertices[v].firstEdgeIndex = permutation[this->vertices[v].firstEdgeIndex];
			}
			
			for(auto edgeId : graphUtil::Range(0, this->adjacencies.size())) {
				auto nextEdgeId = permutation[edgeId];
				while(nextEdgeId != edgeId && nextEdgeId != nullLink) {
					std::swap(this->adjacencies[edgeId], this->adjacencies[nextEdgeId]);
					std::swap(nextEdgeId, permutation[nextEdgeId]);
				}			
				permutation[edgeId] = nextEdgeId;
			}
			freeLinkIndex = nullLink;
			this->adjacencies.resize(lastFreeId);
		}

	private:

		edges_size_type freeLinkIndex;
		AdjacenciesVecType adjacencies;
		VerticesVecType vertices;
		std::unique_ptr<EdgePropertyMapType> edgePropertyMap;
		std::unique_ptr<VertexPropertyMapType> vertexPropertyMap;
	};
}

// PropertyMaps
namespace graph
{
#define DynamicGraphTemplate template<typename VertexProperties, typename EdgeProperties>
#define DynamicGraphType DynamicGraph<VertexProperties, EdgeProperties>

	DynamicGraphTemplate
	struct property_map<DynamicGraphType, vertex_bundle_t> {
		using type = typename DynamicGraphType::VertexPropertyMapType;
	};

	DynamicGraphTemplate
	struct property_map<DynamicGraphType, edge_bundle_t> {
		using type = typename DynamicGraphType::EdgePropertyMapType;
	};

	DynamicGraphTemplate
	inline typename property_map<DynamicGraphType, vertex_bundle_t>::type
	get(const vertex_bundle_t&, DynamicGraphType& graph) {
		return graph.GetVertexPropertyMap();
	}

	DynamicGraphTemplate
	inline typename property_map<DynamicGraphType, edge_bundle_t>::type
	get(const edge_bundle_t&, DynamicGraphType& graph) {
		return graph.GetEdgePropertyMap();
	}

	DynamicGraphTemplate
	struct VertexIndexPropertyMap<DynamicGraphType> {
		using key_type = typename graph_traits<DynamicGraphType>::vertex_descriptor;
		using value_type = typename graph_traits<DynamicGraphType>::vertices_size_type;
		using reference = value_type&;
		using category = boost::readable_property_map_tag;
	};

	DynamicGraphTemplate
	struct property_map<DynamicGraphType, vertex_index_t> {
		using type = VertexIndexPropertyMap<DynamicGraphType>;
	};

	DynamicGraphTemplate
	typename VertexIndexPropertyMap<DynamicGraphType>::value_type
	get(const VertexIndexPropertyMap<DynamicGraphType>& index,
	    const typename VertexIndexPropertyMap<DynamicGraphType>::key_type& key) {
		return key;
	};

	template <typename VertexProperties, typename EdgeProperties>
	inline typename property_map<DynamicGraphType, vertex_index_t>::type
	get(const vertex_index_t&, DynamicGraphType&) {
		return VertexIndexPropertyMap<DynamicGraphType>();
	};

#undef DynamicGraphType
#undef DynamicGraphTemplate
}

#include <graph/dynamic/DynamicGraphTools.hpp>

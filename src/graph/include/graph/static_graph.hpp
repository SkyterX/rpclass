#pragma once
#include <vector>
#include <graph/io/IReader.hpp>
#include <algorithm>
#include <cinttypes>
#include <boost/graph/graph_traits.hpp>
#include <memory>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/property_map/property_map.hpp>
#include "util/Collection.hpp"
#include "properties.hpp"

namespace graph
{
	struct vertex_properties{};
	struct edge_properties{};

	template <typename Iterator>
	class Vertex { // inner struture
	public:
		Iterator begin;
		vertex_properties properties;

		Vertex(const Iterator& it) : begin(it) {}
	};

	template <typename Vertex>
	class Edge {
	public:
		using VertexType = Vertex;
		Vertex source, target;
		
		Edge() {}

		Edge(const Vertex& source, const Vertex& target)
			: source(source),
			target(target) { }
	};

	template <typename Vertex>
	class FancyEdge : public Edge<Vertex> { // inner struture
	public:
		edge_properties properties;

		FancyEdge() {}

		FancyEdge(const Vertex& source, const Vertex& target, const edge_properties& properties = edge_properties())
			: Edge<Vertex>(source, target), properties(properties) {}
	};

	template <typename Vertex>
	class FancyEdgeDescriptor : public Edge<Vertex> { // inner struture
	public:
		edge_properties* properties;

		FancyEdgeDescriptor() : properties(nullptr) {}

		FancyEdgeDescriptor(const Vertex& source, const Vertex& target)
			: Edge<Vertex>(source, target), properties(nullptr) {}

		explicit FancyEdgeDescriptor(FancyEdge<Vertex>& e) 
			: Edge<Vertex>(e.source, e.target), properties(&e.properties) {}

		friend bool operator==(const FancyEdgeDescriptor& lhs, const FancyEdgeDescriptor& rhs) {
			return lhs.properties == rhs.properties;
		}

		friend bool operator!=(const FancyEdgeDescriptor& lhs, const FancyEdgeDescriptor& rhs) {
			return lhs.properties != rhs.properties;
		}
	};

	struct StaticGraphTraversalCategory :
			public boost::bidirectional_graph_tag,
			public boost::vertex_list_graph_tag { };

	class StaticGraph {
		class VertexIterator;
	public:
		class edge_iterator;
		class adjacency_iterator;

		using edge_size_type = uint32_t;
		using vertices_size_type = uint32_t;
		using edges_size_type = uint32_t;
		using degree_size_type = uint16_t;
		using vertex_descriptor = vertices_size_type;
		using directed_category = boost::directed_tag;
		using edge_parallel_category = boost::disallow_parallel_edge_tag;
		using traversal_category = StaticGraphTraversalCategory;

		using EdgeType = FancyEdge<vertex_descriptor>;

	private:
		using AdjacenciesVecType = std::vector<EdgeType*>;
		using AdjacenciesVecIteratorType = AdjacenciesVecType::const_iterator;
	public:
		using out_edge_iterator = edge_iterator;
		using in_edge_iterator = edge_iterator;
		using VertexType = Vertex<AdjacenciesVecIteratorType>;

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
		class VertexPropertyMap;
		class EdgePropertyMap;

		using edge_descriptor = FancyEdgeDescriptor<EdgeType::VertexType>;

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

		const EdgePropertyMap& GetEdgePropertyMap() const;
		const VertexPropertyMap& GetVertexPropertyMap() const;
	private:
		void Initialize();

		AdjacenciesVecType adjacencies;
		VerticesVecType vertices;
		EdgesVecType edges;
		AdjacenciesSeparatorsVecType edgesSeparators;
		std::unique_ptr<EdgePropertyMap> edgePropertyMap;
		std::unique_ptr<VertexPropertyMap> vertexPropertyMap;
		std::unique_ptr<VertexCollection> vertexCollection;
	};

	// adjacency_iterator
	class StaticGraph::adjacency_iterator
			: public boost::iterator_adaptor<
				adjacency_iterator,
				AdjacenciesVecIteratorType,
				vertex_descriptor> {
	public:
		explicit adjacency_iterator(const AdjacenciesVecIteratorType& p, bool isForInEdges)
			: iterator_adaptor_(p), isForInEdges(isForInEdges) {}

	private:
		friend class boost::iterator_core_access;
		friend class edge_iterator;

		vertex_descriptor& dereference() const {
			return isForInEdges ? (*this->base_reference())->source : (*this->base_reference())->target;
		}

		bool isForInEdges;
	};
	
	// edge_iterator
	class StaticGraph::edge_iterator
			: public boost::iterator_adaptor<
				edge_iterator,
				AdjacenciesVecIteratorType,
				edge_descriptor,
				boost::use_default,
				edge_descriptor> {
	public:
		explicit edge_iterator(const AdjacenciesVecIteratorType& p)
			: iterator_adaptor_(p) {}

	private:
		friend class boost::iterator_core_access;

		edge_descriptor dereference() const {
			return edge_descriptor(**this->base_reference());
		}
	};

	class StaticGraph::VertexCollection : public graphUtil::Collection<vertex_iterator> {
	public:
		VertexType operator[](const vertex_descriptor& v) const;
	private:
		friend class StaticGraph;

		explicit VertexCollection(const StaticGraph& g);

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

// VertexPropertyMap
namespace graph
{
	class StaticGraph::VertexPropertyMap {
	private:
		friend class StaticGraph;

		StaticGraph& graph;

		explicit VertexPropertyMap(StaticGraph& graph)
			: graph(graph) {}

	public:
		using key_type = vertex_descriptor;
		using value_type = vertex_properties;
		using reference = value_type&;
		using category = boost::read_write_property_map_tag;

		value_type& get(key_type& key) const {
			return graph.vertices[key].properties;
		}

		void put(const key_type& key, const value_type& value) const {
			graph.vertices[key].properties = value;
		}
	};


	inline StaticGraph::VertexPropertyMap::value_type& get(
		const StaticGraph::VertexPropertyMap& pm,
		StaticGraph::VertexPropertyMap::key_type& key) {
		return pm.get(key);
	}

	inline void put(
		const StaticGraph::VertexPropertyMap& pm,
		const StaticGraph::VertexPropertyMap::key_type& key,
		const StaticGraph::VertexPropertyMap::value_type& value) {
		pm.put(key, value);
	}

//	template<>
//	struct property_map<StaticGraph, vertex_bundle_t> {
//		using type = StaticGraph::VertexPropertyMap;
//	};
//
//	template<>
//	inline property_map<StaticGraph, vertex_bundle_t>::type
//		get<StaticGraph>(const vertex_bundle_t&, StaticGraph& graph) {
//		return graph.GetVertexPropertyMap();
//	}
}

// EdgePropertyMap
namespace graph
{
	class StaticGraph::EdgePropertyMap {
	public:
		using key_type = edge_descriptor;
		using value_type = edge_properties;
		using reference = value_type&;
		using category = boost::read_write_property_map_tag;

		value_type& get(key_type& key) const {
			return *key.properties;
		}

		void put(const key_type& key, const value_type& value) const {
			*key.properties = value;
		}
	};

	inline StaticGraph::EdgePropertyMap::value_type& get(
		const StaticGraph::EdgePropertyMap& pm,
		StaticGraph::EdgePropertyMap::key_type& key) {
		return pm.get(key);
	}

	inline void put(
		const StaticGraph::EdgePropertyMap& pm,
		const StaticGraph::EdgePropertyMap::key_type& key,
		const StaticGraph::EdgePropertyMap::value_type& value) {
		pm.put(key, value);
	}

//	template<>
//	struct property_map<StaticGraph, edge_bundle_t> {
//		using type = StaticGraph::EdgePropertyMap;
//	};
//
//	template<>
//	inline property_map<StaticGraph, edge_bundle_t>::type
//		get<StaticGraph>(const edge_bundle_t&, StaticGraph& graph) {
//		return graph.GetEdgePropertyMap();
//	}
}

// external functions  
namespace graph
{
	inline std::unique_ptr<StaticGraph> ReadGraphFrom(graphIO::IReader& reader) {
		std::unique_ptr<StaticGraph::Builder> builder = nullptr;

		while (char c = reader.NextChar()) {
			switch (c) {
				case 'c': {
					reader.ReadLine();
					break;
				}
				case 'p': {
					reader.NextChar();
					reader.NextChar();

					int vertexCount = reader.NextUnsignedInt() + 1;
					int linksCount = reader.NextUnsignedInt();

					builder = std::make_unique<StaticGraph::Builder>(vertexCount, linksCount);
					break;
				}
				case 'a': {
					int from = reader.NextUnsignedInt();
					int to = reader.NextUnsignedInt();
					int weight = reader.NextUnsignedInt();

					builder->AddEdge(from, to);
					break;
				}
				default:
					throw std::runtime_error("Unexpected token " + c);
			}
		}

		auto graph = builder->Build();

		return graph;
	}

	inline std::pair<StaticGraph::edge_descriptor, bool> edge(StaticGraph::vertex_descriptor u, StaticGraph::vertex_descriptor v, const StaticGraph& g) {
		throw std::exception("Not Supported");
	}
	
	inline std::pair<StaticGraph::vertex_iterator, StaticGraph::vertex_iterator> vertices(const StaticGraph& g) {
		return std::make_pair(g.Vertices().begin(), g.Vertices().end());
	}

	inline std::pair<StaticGraph::adjacency_iterator, StaticGraph::adjacency_iterator> adjacent_vertices(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		auto edgesCollection = g.OutAdjacencies(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	inline StaticGraph::vertex_descriptor source(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.source;
	}

	inline StaticGraph::vertex_descriptor target(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.target;
	}

	inline StaticGraph::degree_size_type out_degree(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		return g.OutAdjacencies(u).size();
	}

	inline std::pair<StaticGraph::out_edge_iterator, StaticGraph::out_edge_iterator> out_edges(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		auto edgesCollection = g.OutEdges(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	inline StaticGraph::vertices_size_type num_vertices(const StaticGraph& g) {
		return g.Vertices().size();
	}

	inline StaticGraph::edges_size_type num_edges(const StaticGraph& g) {
		return g.EdgesCount();
	}

	inline std::pair<StaticGraph::out_edge_iterator, StaticGraph::out_edge_iterator> in_edges(StaticGraph::vertex_descriptor v, const StaticGraph& g) {
		auto edgesCollection = g.InEdges(v);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	inline StaticGraph::degree_size_type in_degree(StaticGraph::vertex_descriptor v, const StaticGraph& g) {
		return g.InAdjacencies(v).size();
	}

	inline StaticGraph::degree_size_type degree(StaticGraph::vertex_descriptor v, const StaticGraph& g) {
		return in_degree(v, g) + out_degree(v, g);
	}
}

// StaticGraph
namespace graph
{
	inline StaticGraph::StaticGraph() : vertexCollection(nullptr), edgePropertyMap(), vertexPropertyMap(new VertexPropertyMap(*this)) {}

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

	inline const StaticGraph::VertexPropertyMap& StaticGraph::GetVertexPropertyMap() const {
		return *vertexPropertyMap;
	}

	inline const StaticGraph::EdgePropertyMap& StaticGraph::GetEdgePropertyMap() const {
		return *edgePropertyMap;
	}

	inline StaticGraph::edges_size_type StaticGraph::EdgesCount() const {
		return edges.size();
	}
}

// VertexCollection  
namespace graph
{
	inline StaticGraph::VertexCollection::VertexCollection(const StaticGraph& g)
		: CollectionType(vertex_iterator(0), vertex_iterator(g.vertices.size())), graph(g) {}

	inline StaticGraph::VertexType StaticGraph::VertexCollection::operator[](const vertex_descriptor& v) const {
		if (v == this->size())
			return VertexType(this->graph.adjacencies.end());
		return this->graph.vertices[v];
	}
}

// Builder
namespace graph
{
	inline StaticGraph::Builder::Builder(vertices_size_type vertexCount, edges_size_type edgesCount) {
		this->unsortedEdges.reserve(edgesCount);
		this->vertexCount = vertexCount;
	}

	inline StaticGraph::Builder::~Builder() {}

	inline void StaticGraph::Builder::AddEdge(const vertex_descriptor& from, const vertex_descriptor& to) {
		unsortedEdges.push_back(EdgeType(from, to));
	}

	inline std::unique_ptr<StaticGraph> StaticGraph::Builder::Build() {
		auto graph = std::make_unique<StaticGraph>();
		BuildGraph(*graph);
		return graph;
	}

	inline void StaticGraph::Builder::BuildGraph(StaticGraph& graph) {
		SortEdgesAndCopyTo(graph);
		graph.Initialize();
	}

	inline void StaticGraph::Builder::SortEdgesAndCopyTo(StaticGraph& graph) {

		graph.vertices.reserve(this->vertexCount);
		graph.edgesSeparators.reserve(this->vertexCount);
		graph.adjacencies.resize(2 * this->unsortedEdges.size());
		graph.edges.reserve(this->unsortedEdges.size());

		auto inDegreeCounts = std::vector<degree_size_type>(this->vertexCount, 0);
		auto outDegreeCounts = std::vector<degree_size_type>(this->vertexCount, 0);
		auto linkPointers = std::vector<AdjacenciesVecType::iterator>();
		linkPointers.reserve(this->vertexCount + 1);

		for (auto& edge : this->unsortedEdges) {
			outDegreeCounts[edge.source]++;
			inDegreeCounts[edge.target]++;
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
			graph.edges.push_back(edge);
			auto edge_ptr = &(graph.edges.back());
			--outDegreeCounts[from];
			*(linkPointers[from] + outDegreeCounts[from]) = edge_ptr;
			--inDegreeCounts[to];
			*(linkPointers[to] + inDegreeCounts[to]) = edge_ptr;
		}

		for (int i = 0; i < this->vertexCount; ++i) {
			auto start = linkPointers[i];
			auto end = linkPointers[i + 1];
			std::sort(start, start + graph.edgesSeparators[i],
			          [](const EdgeType* a, const EdgeType* b) {
				          return a->source < b->source; // in edges are differintiated by source
			          });
			std::sort(start + graph.edgesSeparators[i], end,
			          [](const EdgeType* a, const EdgeType* b) {
				          return a->target < b->target; // out edges are diferentiated by taget
			          });
			start = end;
		}
	}
}

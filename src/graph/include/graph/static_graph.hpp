#pragma once
#include <vector>
#include <graph/io/IReader.hpp>
#include <algorithm>
#include <cinttypes>
#include <boost/graph/graph_traits.hpp>
#include <memory>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include "util/Collection.hpp"

namespace graph
{
	template <typename Iterator>
	class Vertex { // inner struture
	public:
		Iterator begin;

		Vertex(const Iterator& it) : begin(it) {}
	};

	template <typename point_descr>
	class Edge { // inner struture
	public:
		point_descr source, target;

		Edge() { }

		Edge(const point_descr& source, const point_descr& target)
			: source(source),
			  target(target) { }

		friend bool operator==(const Edge& lhs, const Edge& rhs) {
			return lhs.source == rhs.source
					&& lhs.target == rhs.target;
		}

		friend bool operator!=(const Edge& lhs, const Edge& rhs) {
			return !(lhs == rhs);
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

		using EdgeType = Edge<vertex_descriptor>;

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

		using edge_descriptor = EdgeType;

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
	private:
		void Initialize();

		AdjacenciesVecType adjacencies;
		VerticesVecType vertices;
		EdgesVecType edges;
		AdjacenciesSeparatorsVecType edgesSeparators;
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
				EdgeType> {
	public:
		explicit edge_iterator(const AdjacenciesVecIteratorType& p)
			: iterator_adaptor_(p) {}

	private:
		friend class boost::iterator_core_access;

		EdgeType& dereference() const {
			return **this->base_reference();
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
		void AddEdge(const edge_descriptor& e);
		std::unique_ptr<StaticGraph> Build();
	private:
		void BuildGraph(StaticGraph& graph);
		void SortEdgesAndCopyTo(StaticGraph& graph);

		std::vector<edge_descriptor> unsortedEdges;
		int vertexCount;
	};
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

					builder->AddEdge(StaticGraph::EdgeType(from, to));
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
		auto edge = StaticGraph::EdgeType(u, v);
		return std::make_pair(edge, g.OutAdjacencies(u).contains(v));
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
	inline StaticGraph::StaticGraph() : vertexCollection(nullptr) {}

	template <class PairIterator>
	StaticGraph::StaticGraph(PairIterator begin, PairIterator end, vertices_size_type n, edges_size_type m) : StaticGraph() {
		auto builder = new Builder(n, m);

		for (auto& it = begin; it != end; ++it) {
			builder->AddEdge(EdgeType(it->first, it->second));
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

	inline void StaticGraph::Builder::AddEdge(const edge_descriptor& e) {
		unsortedEdges.push_back(e);
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
			outDegreeCounts[source(edge, graph)]++;
			inDegreeCounts[target(edge, graph)]++;
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
			auto from = source(edge, graph);
			auto to = target(edge, graph);
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

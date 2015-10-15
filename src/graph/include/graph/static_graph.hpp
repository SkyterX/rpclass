#pragma once
#include <vector>
#include <graph/io/IReader.hpp>
#include <algorithm>
#include <cinttypes>
#include <boost/graph/graph_traits.hpp>

namespace graph {
	template <typename Iterator>
	class Vertex {
	public:
		Iterator begin;

		Vertex(const Iterator& it) : begin(it) {}
	};

	class StaticGraph {
		class VertexIterator;
	public:
		using edge_size_type = uint32_t;
		using vertices_size_type = uint32_t;
		using edges_size_type = uint32_t;
		using degree_size_type = uint16_t;
		using vertex_descriptor = vertices_size_type;
		using directed_category = boost::directed_tag;
		using edge_parallel_category = boost::disallow_parallel_edge_tag;
		using traversal_category = boost::bidirectional_traversal_tag;

	private:
		using EdgesVecType = std::vector<vertex_descriptor>;
	public:
		using adjacency_iterator = EdgesVecType::const_iterator;
		using out_edge_iterator = adjacency_iterator;
		using VertexType = Vertex<EdgesVecType::const_iterator>;
	private:
		using VerticesVecType = std::vector<VertexType>;
		using EdgesSeparatorsVecType = std::vector<degree_size_type>;
	public:

		class vertex_iterator;
		class Builder;

		class VertexCollection {
		public:
			vertex_iterator begin() const;
			vertex_iterator end() const;
			vertices_size_type size() const;
			VertexType operator[](const vertex_descriptor& v) const;
		private:
			explicit VertexCollection(const StaticGraph& g);
			const StaticGraph& graph;
			friend class StaticGraph;
		};

		class EdgesCollection;

		using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;

		StaticGraph();

		template <class PairIterator>
		StaticGraph(PairIterator begin, PairIterator end,
					vertices_size_type n, edges_size_type m);

		const VertexCollection Vertices;
		EdgesCollection OutEdges(const vertex_descriptor& v) const;
		EdgesCollection InEdges(const vertex_descriptor& v) const;
		edges_size_type EdgesCount() const;
	private:
		EdgesVecType edges;
		VerticesVecType vertices;
		EdgesSeparatorsVecType edgesSeparators;
	};

	class StaticGraph::EdgesCollection {
	public:
		adjacency_iterator begin() const;
		adjacency_iterator end() const;
		degree_size_type size() const;
		bool contains(vertex_descriptor v) const;
	private:
		EdgesCollection(const StaticGraph& g, const vertex_descriptor& v, const bool& forInEdges);
		const StaticGraph& graph;
		const vertex_descriptor& vertex;
		const bool isForInEdges;
		friend class StaticGraph;
	};

	class StaticGraph::vertex_iterator : std::iterator<std::forward_iterator_tag, vertex_descriptor> {
	public:
		vertex_iterator(const StaticGraph& g, vertex_descriptor start);
		explicit vertex_iterator(const StaticGraph& g);
		virtual ~vertex_iterator();
		void swap(vertex_iterator& other);
		vertex_iterator& operator++();
		vertex_iterator operator++(int);
		bool operator==(const vertex_iterator& other) const;
		bool operator!=(const vertex_iterator& other) const;
		const vertex_descriptor& operator*() const;
		const vertex_descriptor* operator->() const;
	private:
		vertex_descriptor currentVertex;
		vertex_descriptor verticesCount;
	};

	class StaticGraph::Builder {
		friend class StaticGraph;

	public:
		Builder(vertices_size_type vertexCount, edges_size_type edgesCount);
		virtual ~Builder();
		void AddEdge(const edge_descriptor& e);
		StaticGraph* Build();
	private:
		void SortEdgesAndCopyTo(StaticGraph& graph);

		std::vector<edge_descriptor> unsortedEdges;
		int vertexCount;
	};
}

// external functions  
namespace graph {
	inline StaticGraph* ReadGraphFrom(graphIO::IReader& reader) {
		StaticGraph::Builder* builder = nullptr;

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

					builder = new StaticGraph::Builder(vertexCount, linksCount);
					break;
				}
				case 'a': {
					int from = reader.NextUnsignedInt();
					int to = reader.NextUnsignedInt();
					int weight = reader.NextUnsignedInt();

					builder->AddEdge(std::make_pair(from, to));
					break;
				}
				default:
					throw std::runtime_error("Unexpected token " + c);
			}
		}

		auto graph = builder->Build();

		delete builder;
		return graph;
	}

	inline std::pair<StaticGraph::edge_descriptor, bool> edge(StaticGraph::vertex_descriptor u, StaticGraph::vertex_descriptor v, const StaticGraph& g) {
		auto edge = std::make_pair(u, v);
		return make_pair(edge, g.OutEdges(u).contains(v));
	}


	inline std::pair<StaticGraph::vertex_iterator, StaticGraph::vertex_iterator> vertices(const StaticGraph& g) {
		return make_pair(g.Vertices.begin(), g.Vertices.end());
	}

	inline std::pair<StaticGraph::adjacency_iterator, StaticGraph::adjacency_iterator> adjacent_vertices(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		auto edgesCollection = g.OutEdges(u);
		return make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	inline StaticGraph::vertex_descriptor source(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.first;
	}

	inline StaticGraph::vertex_descriptor target(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.second;
	}

	inline StaticGraph::degree_size_type out_degree(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		return g.OutEdges(u).size();
	}

	inline StaticGraph::vertices_size_type num_vertices(const StaticGraph& g) {
		return g.Vertices.size();
	}

	inline StaticGraph::edges_size_type num_edges(const StaticGraph& g) {
		return g.EdgesCount() / 2;
	}
}

// StaticGraph
namespace graph {
	inline StaticGraph::StaticGraph() : Vertices(*this) {}

	template <class PairIterator>
	StaticGraph::StaticGraph(PairIterator begin, PairIterator end, vertices_size_type n, edges_size_type m) : StaticGraph() {
		auto builder = new Builder(n, m);

		for (auto& it = begin; it != end; ++it) {
			builder->AddEdge(*it);
		}

		builder->SortEdgesAndCopyTo(*this);

		delete builder;
	}

	inline StaticGraph::EdgesCollection StaticGraph::InEdges(const vertex_descriptor& v) const {
		return EdgesCollection(*this, v, true);
	}

	inline StaticGraph::EdgesCollection StaticGraph::OutEdges(const vertex_descriptor& v) const {
		return EdgesCollection(*this, v, false);
	}

	inline StaticGraph::edges_size_type StaticGraph::EdgesCount() const {
		return edges.size();
	}
}

// vertex_iterator  
namespace graph {
	inline StaticGraph::vertex_iterator::vertex_iterator(const StaticGraph& g, vertex_descriptor start)
		: currentVertex(start), verticesCount(g.vertices.size()) {}

	inline StaticGraph::vertex_iterator::vertex_iterator(const StaticGraph& g)
		: currentVertex(g.vertices.size()), verticesCount(g.vertices.size()) {}

	inline StaticGraph::vertex_iterator::~vertex_iterator() {}

	inline void StaticGraph::vertex_iterator::swap(vertex_iterator& other) {
		std::swap(currentVertex, other.currentVertex);
	}

	inline StaticGraph::vertex_iterator& StaticGraph::vertex_iterator::operator++() {
		if (currentVertex != verticesCount)
			++currentVertex;
		return *this;
	}

	inline StaticGraph::vertex_iterator StaticGraph::vertex_iterator::operator++(int) {
		if (currentVertex == verticesCount)
			return *this;
		vertex_iterator tmp(*this);
		++currentVertex;
		return tmp;
	}

	inline bool StaticGraph::vertex_iterator::operator==(const vertex_iterator& other) const {
		return currentVertex == other.currentVertex;
	}

	inline bool StaticGraph::vertex_iterator::operator!=(const vertex_iterator& other) const {
		return currentVertex != other.currentVertex;
	}

	inline const StaticGraph::vertex_descriptor& StaticGraph::vertex_iterator::operator*() const {
		return currentVertex;
	}

	inline const StaticGraph::vertex_descriptor* StaticGraph::vertex_iterator::operator->() const {
		return &currentVertex;
	}
}

// VertexCollection  
namespace graph {
	inline StaticGraph::VertexCollection::VertexCollection(const StaticGraph& g) : graph(g) {}

	inline StaticGraph::vertex_iterator StaticGraph::VertexCollection::begin() const {
		return vertex_iterator(this->graph, 0);
	}

	inline StaticGraph::vertex_iterator StaticGraph::VertexCollection::end() const {
		return vertex_iterator(this->graph);
	}

	inline StaticGraph::vertices_size_type StaticGraph::VertexCollection::size() const {
		return this->graph.vertices.size();
	}

	inline StaticGraph::VertexType StaticGraph::VertexCollection::operator[](const vertex_descriptor& v) const {
		if (v == this->size())
			return VertexType(this->graph.edges.end());
		return this->graph.vertices[v];
	}
}

// EdgesCollection  
namespace graph {
	inline StaticGraph::EdgesCollection::EdgesCollection(const StaticGraph& g, const vertex_descriptor& v, const bool& forInEdges)
		:graph(g), vertex(v), isForInEdges(forInEdges) {}

	inline StaticGraph::adjacency_iterator StaticGraph::EdgesCollection::begin() const {
		return graph.Vertices[vertex].begin + (isForInEdges ? 0 : graph.edgesSeparators[vertex]);
	}

	inline StaticGraph::adjacency_iterator StaticGraph::EdgesCollection::end() const {
		return isForInEdges
			? graph.Vertices[vertex].begin + graph.edgesSeparators[vertex]
			: graph.Vertices[vertex + 1].begin;
	}

	inline StaticGraph::degree_size_type StaticGraph::EdgesCollection::size() const {
		return (degree_size_type)(end() - begin());
	}

	inline bool StaticGraph::EdgesCollection::contains(vertex_descriptor v) const {
		auto result = lower_bound(this->begin(), this->end(), v);
		return result != this->end() && *result == v;
	}
}

// Builder
namespace graph {
	inline StaticGraph::Builder::Builder(vertices_size_type vertexCount, edges_size_type edgesCount) {
		this->unsortedEdges.reserve(edgesCount);
		this->vertexCount = vertexCount;
	}

	inline StaticGraph::Builder::~Builder() {}

	inline void StaticGraph::Builder::AddEdge(const edge_descriptor& e) {
		unsortedEdges.push_back(e);
	}

	inline StaticGraph* StaticGraph::Builder::Build() {
		StaticGraph* graph = new StaticGraph();
		SortEdgesAndCopyTo(*graph);
		return graph;
	}

	inline void StaticGraph::Builder::SortEdgesAndCopyTo(StaticGraph& graph) {

		graph.vertices.reserve(this->vertexCount);
		graph.edgesSeparators.reserve(this->vertexCount);
		graph.edges.resize(2 * this->unsortedEdges.size());

		auto inDegreeCounts = std::vector<degree_size_type>(this->vertexCount, 0);
		auto outDegreeCounts = std::vector<degree_size_type>(this->vertexCount, 0);
		auto linksPointers = std::vector<EdgesVecType::iterator>();
		linksPointers.reserve(this->vertexCount+1);

		for (auto& edge : this->unsortedEdges) {
			outDegreeCounts[source(edge, graph)]++;
			inDegreeCounts[target(edge, graph)]++;
		}

		linksPointers.push_back(graph.edges.begin());
		graph.vertices.push_back(VertexType(linksPointers[0]));
		graph.edgesSeparators.push_back(inDegreeCounts[0]);
		outDegreeCounts[0] += inDegreeCounts[0];
		for (int i = 1; i < this->vertexCount; ++i) {
			linksPointers.push_back(linksPointers[i - 1] + outDegreeCounts[i - 1]);
			graph.vertices.push_back(VertexType(linksPointers[i]));
			graph.edgesSeparators.push_back(inDegreeCounts[i]);
			outDegreeCounts[i] += inDegreeCounts[i];
		}
		linksPointers.push_back(graph.edges.end());

		for (auto& edge : this->unsortedEdges) {
			auto from = source(edge, graph);
			auto to = target(edge, graph);
			--outDegreeCounts[from];
			*(linksPointers[from] + outDegreeCounts[from]) = to;
			--inDegreeCounts[to];
			*(linksPointers[to] + inDegreeCounts[to]) = from;
		}

		for (int i = 0; i < this->vertexCount; ++i) {
			auto start = linksPointers[i];
			auto end = linksPointers[i + 1];
			sort(start, start + graph.edgesSeparators[i]);
			sort(start + graph.edgesSeparators[i], end);
			start = end;
		}
	}
}

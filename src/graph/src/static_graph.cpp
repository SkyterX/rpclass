#include <graph/static_graph.hpp>
#include <algorithm>

// vertex_iterator  
namespace graph {

	StaticGraph::vertex_iterator::vertex_iterator(const StaticGraph& g, vertex_descriptor start)
		: currentVertex(start), verticesCount(g.vertices.size()) {}

	StaticGraph::vertex_iterator::vertex_iterator(const StaticGraph& g)
		: currentVertex(g.vertices.size()), verticesCount(g.vertices.size()) {}

	StaticGraph::vertex_iterator::~vertex_iterator() {}

	void StaticGraph::vertex_iterator::swap(vertex_iterator& other) {
		std::swap(currentVertex, other.currentVertex);
	}

	StaticGraph::vertex_iterator& StaticGraph::vertex_iterator::operator++() {
		if (currentVertex != verticesCount)
			++currentVertex;
		return *this;
	}

	StaticGraph::vertex_iterator StaticGraph::vertex_iterator::operator++(int) {
		if (currentVertex == verticesCount)
			return *this;
		vertex_iterator tmp(*this);
		++currentVertex;
		return tmp;
	}

	bool StaticGraph::vertex_iterator::operator==(const vertex_iterator& other) const {
		return currentVertex == other.currentVertex;
	}

	bool StaticGraph::vertex_iterator::operator!=(const vertex_iterator& other) const {
		return currentVertex != other.currentVertex;
	}

	const StaticGraph::vertex_descriptor& StaticGraph::vertex_iterator::operator*() const {
		return currentVertex;
	}

	const StaticGraph::vertex_descriptor* StaticGraph::vertex_iterator::operator->() const {
		return &currentVertex;
	}
}

// Builder
namespace graph {

	StaticGraph::Builder::Builder(vertices_size_type vertexCount, edges_size_type edgesCount) {
		this->unsortedEdges.reserve(edgesCount);
		this->vertexCount = vertexCount;
	}

	StaticGraph::Builder::~Builder() {}

	void StaticGraph::Builder::AddEdge(const edge_descriptor& e) {
		unsortedEdges.push_back(e);
	}

	StaticGraph* StaticGraph::Builder::Build() {
		StaticGraph* graph = new StaticGraph();
		SortEdgesAndCopyTo(*graph);
		return graph;
	}

	void StaticGraph::Builder::SortEdgesAndCopyTo(StaticGraph& graph) {

		graph.vertices.reserve(this->vertexCount);
		graph.edges.resize(this->unsortedEdges.size());

		auto degreeCounts = std::vector<degree_size_type>(this->vertexCount, 0);
		auto linksPointers = std::vector<EdgesVecType::iterator>();
		linksPointers.reserve(this->vertexCount);

		for (auto& edge : this->unsortedEdges) {
			degreeCounts[source(edge, graph)]++;
		}

		linksPointers.push_back(graph.edges.begin());
		graph.vertices.push_back(VertexType(linksPointers[0]));
		for (int i = 1; i < this->vertexCount; ++i) {
			linksPointers.push_back(linksPointers[i - 1] + degreeCounts[i - 1]);
			graph.vertices.push_back(VertexType(linksPointers[i]));
		}

		for (auto& edge : this->unsortedEdges) {
			*(linksPointers[source(edge, graph)]++) = target(edge, graph);
		}

		auto start = graph.edges.begin();
		for (int i = 0; i < this->vertexCount; ++i) {
			auto end = linksPointers[i];
			sort(start, end);
			start = end;
		}
	}
}

// external functions  
namespace graph {
	StaticGraph* ReadGraphFrom(IReader& reader) {
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

	std::pair<StaticGraph::edge_descriptor, bool> edge(StaticGraph::vertex_descriptor u, StaticGraph::vertex_descriptor v, const StaticGraph& g) {
		auto adjacencyIterators = adjacent_vertices(u, g);
		auto result = lower_bound(adjacencyIterators.first, adjacencyIterators.second, v);

		StaticGraph::edge_descriptor edge = std::make_pair(u, v);
		return make_pair(edge, result != adjacencyIterators.second && *result == v);
	}

	template <class PairIterator>
	StaticGraph::StaticGraph(PairIterator begin, PairIterator end, vertices_size_type n, edges_size_type m) {
		auto builder = new Builder(n, m);

		for (auto it = begin; it != end; ++it) {
			builder->AddEdge(it);
		}

		builder->SortEdgesAndCopyTo(*this);

		delete builder;
	}

	std::pair<StaticGraph::vertex_iterator, StaticGraph::vertex_iterator> vertices(const StaticGraph& g) {
		return make_pair(StaticGraph::vertex_iterator(g, 0), StaticGraph::vertex_iterator(g));
	}

	std::pair<StaticGraph::adjacency_iterator, StaticGraph::adjacency_iterator> adjacent_vertices(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		return make_pair(g.vertices[u].begin, g.vertices[u].begin + out_degree(u, g));
	}

	StaticGraph::vertex_descriptor source(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.first;
	}

	StaticGraph::vertex_descriptor target(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.second;
	}

	StaticGraph::degree_size_type out_degree(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		if (u == g.vertices.size() - 1)
			return (StaticGraph::degree_size_type)(g.edges.end() - g.vertices[u].begin);
		return (StaticGraph::degree_size_type)(g.vertices[u + 1].begin - g.vertices[u].begin);
	}

	StaticGraph::vertices_size_type num_vertices(const StaticGraph& g) {
		return g.vertices.size();
	}

	StaticGraph::edges_size_type num_edges(const StaticGraph& g) {
		return g.edges.size();
	}
}

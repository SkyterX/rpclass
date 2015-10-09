#include <graph/static_graph.hpp>
#include <algorithm>

// StaticGraph
namespace graph
{
	StaticGraph::StaticGraph() : Vertices(*this) {}

	StaticGraph::EdgesCollection StaticGraph::Edges(const vertex_descriptor& v) const {
		return EdgesCollection(*this, v);
	}

	StaticGraph::edges_size_type StaticGraph::EdgesCount() const {
		return edges.size();
	}

	template <class PairIterator>
	StaticGraph::StaticGraph(PairIterator begin, PairIterator end, vertices_size_type n, edges_size_type m) {
		auto builder = new Builder(n, m);

		for (auto& it = begin; it != end; ++it) {
			builder->AddEdge(it);
		}

		builder->SortEdgesAndCopyTo(*this);

		delete builder;
	}
}

// vertex_iterator  
namespace graph
{
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

// VertexCollection  
namespace graph
{
	StaticGraph::VertexCollection::VertexCollection(const StaticGraph& g) : graph(g) {}

	StaticGraph::vertex_iterator StaticGraph::VertexCollection::begin() const {
		return vertex_iterator(this->graph, 0);
	}

	StaticGraph::vertex_iterator StaticGraph::VertexCollection::end() const {
		return vertex_iterator(this->graph);
	}

	StaticGraph::vertices_size_type StaticGraph::VertexCollection::size() const {
		return this->graph.vertices.size();
	}

	StaticGraph::VertexType StaticGraph::VertexCollection::operator[](const vertex_descriptor& v) const {
		if (v == this->size())
			return VertexType(this->graph.edges.end());
		return this->graph.vertices[v];
	}
}

// EdgesCollection  
namespace graph
{
	StaticGraph::EdgesCollection::EdgesCollection(const StaticGraph& g, const vertex_descriptor& v)
		:graph(g), vertex(v) {}

	StaticGraph::adjacency_iterator StaticGraph::EdgesCollection::begin() const{
		return this->graph.Vertices[this->vertex].begin;
	}

	StaticGraph::adjacency_iterator StaticGraph::EdgesCollection::end() const{
		return this->graph.Vertices[this->vertex + 1].begin;
	}

	StaticGraph::degree_size_type StaticGraph::EdgesCollection::size() const{
		return (degree_size_type)(end() - begin());
	}
	
	bool StaticGraph::EdgesCollection::contains(vertex_descriptor v) const {
		auto result = lower_bound(this->begin(), this->end(), v);
		return result != this->end() && *result == v;
	}
}

// Builder
namespace graph
{
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
namespace graph
{
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
		auto edge = std::make_pair(u, v);
		return make_pair(edge, g.Edges(u).contains(v));
	}


	std::pair<StaticGraph::vertex_iterator, StaticGraph::vertex_iterator> vertices(const StaticGraph& g) {
		return make_pair(g.Vertices.begin(), g.Vertices.end());
	}

	std::pair<StaticGraph::adjacency_iterator, StaticGraph::adjacency_iterator> adjacent_vertices(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		auto edgesCollection = g.Edges(u);
		return make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	StaticGraph::vertex_descriptor source(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.first;
	}

	StaticGraph::vertex_descriptor target(StaticGraph::edge_descriptor e, const StaticGraph& g) {
		return e.second;
	}

	StaticGraph::degree_size_type out_degree(StaticGraph::vertex_descriptor u, const StaticGraph& g) {
		return g.Edges(u).size();
	}

	StaticGraph::vertices_size_type num_vertices(const StaticGraph& g) {
		return g.Vertices.size();
	}

	StaticGraph::edges_size_type num_edges(const StaticGraph& g) {
		return g.EdgesCount();
	}
}

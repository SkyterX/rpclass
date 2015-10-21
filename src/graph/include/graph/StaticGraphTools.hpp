#pragma once
#include "static_graph.hpp"
#include <graph/io/IReader.hpp>

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
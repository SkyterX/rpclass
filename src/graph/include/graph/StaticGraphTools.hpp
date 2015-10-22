#pragma once
#include "static_graph.hpp"
#include <graph/io/IReader.hpp>

namespace graph {
#define Properties VertexProperties, EdgeProperties

	template <typename VertexProperties = NoProperties, typename EdgeProperties = NoProperties>
	inline std::unique_ptr<StaticGraph<Properties>> ReadGraphFrom(graphIO::IReader& reader) {
		std::unique_ptr<typename StaticGraph<Properties>::Builder> builder = nullptr;

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

					builder = std::make_unique<typename StaticGraph<Properties>::Builder>(vertexCount, linksCount);
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

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::edge_descriptor, bool> edge(
		typename StaticGraph<Properties>::vertex_descriptor,
		typename StaticGraph<Properties>::vertex_descriptor,
		const StaticGraph<Properties>&) {
		throw std::exception("Not Supported");
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::vertex_iterator, typename StaticGraph<Properties>::vertex_iterator>
	vertices(const StaticGraph<Properties>& g) {
		return std::make_pair(g.Vertices().begin(), g.Vertices().end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::adjacency_iterator, typename StaticGraph<Properties>::adjacency_iterator>
	adjacent_vertices(typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.OutAdjacencies(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::in_adjacency_iterator, typename StaticGraph<Properties>::in_adjacency_iterator>
	in_adjacent_vertices(typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.InAdjacencies(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::vertex_descriptor source(
		typename StaticGraph<Properties>::edge_descriptor e, const StaticGraph<Properties>&) {
		return e.source;
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::vertex_descriptor target(
		typename StaticGraph<Properties>::edge_descriptor e, const StaticGraph<Properties>&) {
		return e.target;
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::degree_size_type out_degree(
		typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		return g.OutAdjacencies(u).size();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::out_edge_iterator, typename StaticGraph<Properties>::out_edge_iterator>
	out_edges(typename StaticGraph<Properties>::vertex_descriptor u, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.OutEdges(u);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::vertices_size_type num_vertices(const StaticGraph<Properties>& g) {
		return g.Vertices().size();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::edges_size_type num_edges(const StaticGraph<Properties>& g) {
		return g.EdgesCount();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline std::pair<typename StaticGraph<Properties>::out_edge_iterator, typename StaticGraph<Properties>::out_edge_iterator>
	in_edges(typename StaticGraph<Properties>::vertex_descriptor v, const StaticGraph<Properties>& g) {
		auto edgesCollection = g.InEdges(v);
		return std::make_pair(edgesCollection.begin(), edgesCollection.end());
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::degree_size_type in_degree(
		typename StaticGraph<Properties>::vertex_descriptor v, const StaticGraph<Properties>& g) {
		return g.InAdjacencies(v).size();
	}

	template <typename VertexProperties, typename EdgeProperties>
	inline typename StaticGraph<Properties>::degree_size_type degree(
		typename StaticGraph<Properties>::vertex_descriptor v, const StaticGraph<Properties>& g) {
		return in_degree(v, g) + out_degree(v, g);
	}
#undef Properties
}

#pragma once

namespace graph
{
	struct vertex_properties_stub {};

	struct edge_properties_stub {};

	template <typename Iterator>
	class Vertex { // inner struture
	public:
		Iterator begin;
		vertex_properties_stub properties;

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
		edge_properties_stub properties;

		FancyEdge() {}

		FancyEdge(const Vertex& source, const Vertex& target, const edge_properties_stub& properties = edge_properties_stub())
			: Edge<Vertex>(source, target), properties(properties) {}
	};

	template <typename Vertex>
	class FancyEdgeDescriptor : public Edge<Vertex> { // inner struture
	public:
		edge_properties_stub* properties;

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
}

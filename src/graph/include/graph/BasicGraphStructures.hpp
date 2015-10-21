#pragma once

namespace graph
{
	template <typename Iterator, typename VertexProperties>
	class Vertex { // inner struture
	public:
		using IteratorType = Iterator;

		Iterator begin;
		VertexProperties properties;

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

	template <typename Vertex, typename EdgeProperties>
	class FancyEdge : public Edge<Vertex> { // inner struture
	public:
		using EdgePropertiesType = EdgeProperties;

		EdgeProperties properties;

		FancyEdge() {}

		FancyEdge(const Vertex& source, const Vertex& target, const EdgeProperties& properties = EdgeProperties())
			: Edge<Vertex>(source, target), properties(properties) {}
	};

	template <typename Vertex, typename EdgeProperties>
	class FancyEdgeDescriptor : public Edge<Vertex> { // inner struture
	public:
		using EdgePropertiesType = EdgeProperties;

		EdgePropertiesType* properties;

		FancyEdgeDescriptor() : properties(nullptr) {}

		FancyEdgeDescriptor(const Vertex& source, const Vertex& target)
			: Edge<Vertex>(source, target), properties(nullptr) {}

		explicit FancyEdgeDescriptor(FancyEdge<Vertex, EdgeProperties>& e)
			: Edge<Vertex>(e.source, e.target), properties(&e.properties) {}

		friend bool operator==(const FancyEdgeDescriptor& lhs, const FancyEdgeDescriptor& rhs) {
			return lhs.properties == rhs.properties;
		}

		friend bool operator!=(const FancyEdgeDescriptor& lhs, const FancyEdgeDescriptor& rhs) {
			return lhs.properties != rhs.properties;
		}
	};
}

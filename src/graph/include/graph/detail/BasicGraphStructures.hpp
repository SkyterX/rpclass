#pragma once

namespace graph {
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

		FancyEdgeDescriptor(const Vertex& source, const Vertex& target, EdgeProperties* properties)
			: Edge<Vertex>(source, target), properties(properties) {}

		friend bool operator==(const FancyEdgeDescriptor& lhs, const FancyEdgeDescriptor& rhs) {
			return lhs.properties == rhs.properties;
		}

		friend bool operator!=(const FancyEdgeDescriptor& lhs, const FancyEdgeDescriptor& rhs) {
			return lhs.properties != rhs.properties;
		}
	};

	template <typename Vertex>
	class Link {
	public:
		using VertexType = Vertex;

		Vertex target;

		Link() {}

		explicit Link(const Vertex& target)
			: target(target) {}
	};

	template <typename Vertex, typename EdgeProperties>
	class FancyLink : public Link<Vertex> {
	public:
		using EdgePropertiesType = EdgeProperties;

		EdgePropertiesType* properties;

		FancyLink() : properties(nullptr) {}


		explicit FancyLink(const Vertex& target)
			: Link<Vertex>(target), properties(nullptr) {}


		FancyLink(const Vertex& target, EdgeProperties* properties)
			:Link<Vertex>(target), properties(properties) {}
	};
}

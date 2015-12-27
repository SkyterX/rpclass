#pragma once
#include <limits>
#include <graph/detail/BasicGraphStructures.hpp>

namespace graph
{
	template <typename Graph>
	class DGVertex { // inner struture
		using EdgesSizeType = typename Graph::edges_size_type;
		using DegreeType = typename Graph::degree_size_type;
		using VertexProperties = typename Graph::vertex_bundled;
		const static EdgesSizeType nullLink = std::numeric_limits<EdgesSizeType>::max();
	public:
		DegreeType degree;
		size_t firstEdgeIndex;
		VertexProperties properties;

		DGVertex(const VertexProperties& properties = VertexProperties(), EdgesSizeType firstEdgeIndex = nullLink)
			: degree(0), firstEdgeIndex(firstEdgeIndex), properties(properties) {}
	};

	template <typename Vertex, typename EdgeProperties>
	class DGFancyLink : public Link<Vertex> {
		using BaseType = Link<Vertex>;

		const static size_t nullLink = std::numeric_limits<size_t>::max();
	public:
		using EdgePropertiesType = EdgeProperties;

		size_t nextLink;
		EdgePropertiesType properties;

		DGFancyLink() : BaseType(), nextLink(nullLink) {}

		explicit DGFancyLink(const Vertex& target, size_t nextLink = nullLink)
			: BaseType(target), nextLink(nextLink) {}


		DGFancyLink(const Vertex& target, const EdgeProperties& properties, size_t nextLink = nullLink)
			:BaseType(target), properties(properties), nextLink(nextLink) {}
	};
}

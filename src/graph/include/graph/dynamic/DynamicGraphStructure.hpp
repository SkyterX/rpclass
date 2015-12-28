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
		EdgesSizeType firstEdgeIndex;
		VertexProperties properties;

		DGVertex(const VertexProperties& properties = VertexProperties(), EdgesSizeType firstEdgeIndex = nullLink)
			: degree(0), firstEdgeIndex(firstEdgeIndex), properties(properties) {}
	};

	template <typename Graph>
	class DGFancyLink : public Link<typename Graph::vertex_descriptor> {
		using BaseType = Link<typename Graph::vertex_descriptor>;
		using Vertex = typename Graph::vertex_descriptor;
		using EdgeProperties = typename Graph::edge_bundled;
		using EdgesSizeType = typename Graph::edges_size_type;

		const static EdgesSizeType nullLink = std::numeric_limits<EdgesSizeType>::max();
	public:		
		using EdgePropertiesType = EdgeProperties;

		EdgesSizeType nextLink;
		EdgePropertiesType properties;

		DGFancyLink() : BaseType(), nextLink(nullLink) {}

		explicit DGFancyLink(const Vertex& target, EdgesSizeType nextLink = nullLink)
			: BaseType(target), nextLink(nextLink) {}


		DGFancyLink(const Vertex& target, const EdgeProperties& properties, EdgesSizeType nextLink = nullLink)
			:BaseType(target), nextLink(nextLink), properties(properties) {}
	};
}

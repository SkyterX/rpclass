#pragma once

#include <graph/static_graph.hpp>
#include <cinttypes>
#include <graph/io/IReader.hpp>
#include <graph/properties.hpp>

namespace graphOSM {
	struct WeightTag {};
	using OSMWeightType = int32_t;

	using OSMEdgeProperties = graph::Properties<graph::Property<WeightTag, OSMWeightType>>;
	using OSMVertexProperties = graph::NoProperties;
	using OSMGraph = graph::StaticGraph<OSMVertexProperties, OSMEdgeProperties>;

	inline std::unique_ptr<OSMGraph> ReadGraphFrom(graphIO::IReader& reader) {

		std::unique_ptr<OSMGraph::Builder> builder = nullptr;

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

				builder = std::make_unique<OSMGraph::Builder>(vertexCount, linksCount);
				break;
			}
			case 'a': {
				int from = reader.NextUnsignedInt();
				int to = reader.NextUnsignedInt();
				int weight = reader.NextUnsignedInt();

				OSMEdgeProperties edgeProperties;
				graph::get<WeightTag>(edgeProperties) = weight;

				builder->AddEdge(from, to, edgeProperties);
				break;
			}
			default:
				throw std::runtime_error("Unexpected token ");
			}
		}

		auto graph = builder->Build();

		return graph;
	}
}
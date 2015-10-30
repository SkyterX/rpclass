#pragma once

#include <graph/static_graph.hpp>
#include <cinttypes>
#include <graph/io/IReader.hpp>
#include <graph/properties.hpp>
#include <fstream>
#include <graph/io/FileReader.hpp>
#include <graph/io/IReader.hpp>



namespace graphOSM {
	struct WeightTag {};
	using OSMWeightType = int32_t;

	using OSMEdgeProperties = graph::Properties<graph::Property<WeightTag, OSMWeightType>>;
	using OSMVertexProperties = graph::NoProperties;
	using OSMGraph = graph::StaticGraph<OSMVertexProperties, OSMEdgeProperties>;

	enum Direction {
		DIR_BOTH = 0, DIR_TAIL_HEAD = 2, DIR_HEAD_TAIL = 1, DIR_CLOSED = 3
	};

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

	inline std::unique_ptr<OSMGraph> ReadGraphFromDdsg(graphIO::IReader& reader) {
		// read edge file header
		std::string line = reader.ReadLine();
		if (line != "d") {
			std::cout << "expecting input file to start with a 'd'." << std::endl;
			abort();
		}

		// read number of vertices and edges
		graph::graph_traits<OSMGraph>::vertices_size_type vertexCount = reader.NextUnsignedInt();
		graph::graph_traits<OSMGraph>::edges_size_type linksCount = reader.NextUnsignedInt();
		std::cout << "ddsg graph has " << vertexCount << " nodes and " << linksCount << " edges." << std::endl;

		std::unique_ptr<OSMGraph::Builder> builder = std::make_unique<OSMGraph::Builder>(vertexCount, linksCount);

		while (reader.HasNext()) {
			// each line = 4 ints = (from to weight direction)
			uint32_t from = reader.NextUnsignedInt();
			uint32_t to = reader.NextUnsignedInt();
			uint32_t weight = reader.NextUnsignedInt();
			uint32_t direction = reader.NextUnsignedInt();
			OSMEdgeProperties edgeProperties;
			graph::get<WeightTag>(edgeProperties) = weight;
			if(direction == DIR_BOTH || direction == DIR_HEAD_TAIL) {
				builder->AddEdge(from, to, edgeProperties);				
			}
			if(direction == DIR_BOTH || direction == DIR_TAIL_HEAD) {
				builder->AddEdge(to, from, edgeProperties);
			}
		}
		return builder->Build();
	}
}
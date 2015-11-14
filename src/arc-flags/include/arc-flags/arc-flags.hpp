#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <graph/static_graph.hpp>
#include <graph/dijkstra.hpp>
#include <arc-flags/Bitset.hpp>
#include <graph/io/FileReader.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/detail/ComplementGraph.hpp>
#include <arc-flags/arc-flagsReduction.hpp>

namespace arcflags {
	template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
			  typename IndexMapTag, typename ColorMapTag, typename ArcFlagsMapTag,
			  typename PartitionMapTag, size_t N,
			  typename BundledVertexProperties, typename BundledEdgeProperties>
	struct GenerateArcFlagsGraph {};

	template <typename PredecessorMapTag, class DisanceMapTag, typename WeightMapTag,
			  typename IndexMapTag, typename ColorMapTag, typename ArcFlagsMapTag,
			  typename PartitionMapTag, size_t N, typename... P1s, typename... P2s>
	struct GenerateArcFlagsGraph<PredecessorMapTag, DisanceMapTag, WeightMapTag,
								 IndexMapTag, ColorMapTag, ArcFlagsMapTag, PartitionMapTag, N,
								 graph::Properties<P1s...>, graph::Properties<P2s...>> {
		using type = graph::StaticGraph<
			graph::Properties<
				graph::Property<PredecessorMapTag,
								typename graph::graph_traits<graph::StaticGraph<graph::Properties<>, graph::Properties<>>>::vertex_descriptor>,
				graph::Property<DisanceMapTag, uint32_t>,
				graph::Property<ColorMapTag, char>,
				graph::Property<PartitionMapTag, char>,
				P1s...>,
			graph::Properties<
				graph::Property<WeightMapTag, uint32_t>,
				graph::Property<ArcFlagsMapTag, bitset::Bitset<N>>,
				P2s...>>;
	};

	// read partitionining from a file
	template <size_t N, typename PartitionMapTag, typename Graph>
	int read_partitioning(Graph& graph, const char* PathToFile) {
		graphIO::FileReader fileReader;
		fileReader.Open(PathToFile);
		auto partitionMap = get(PartitionMapTag(), graph);

		for (auto& v : graphUtil::Range(graph::vertices(graph))) {
			auto classIndex = fileReader.NextUnsignedInt();
			put(partitionMap, v, static_cast<char>(classIndex));
		}

		fileReader.Close();

		return 0;
	};

	template <size_t N, typename Graph, typename ArcFlagsMap>
	int read_arcflags(Graph& graph, ArcFlagsMap& arcflags, const char* PathToFile) {
		using namespace graph;
		using namespace graphIO;
		using namespace graphUtil;
		using namespace std;

		FileReader fileReader;
		if (!fileReader.Open(PathToFile))
			return 1;

		for (const auto& v : Range(vertices(graph))) {
			for (const auto& outEdge : Range(out_edges(v, graph))) {
				assert(fileReader.HasNext());
				auto start = fileReader.NextUnsignedInt();
				auto end = fileReader.NextUnsignedInt();
				assert(start == source(outEdge, graph));
				assert(end == target(outEdge, graph));
				auto& arcFlag = get(arcflags, outEdge);
				for (const auto& bitIndex : Range(0, N)) {
					char bit = fileReader.NextChar();
					arcFlag.SetBit(bitIndex, bit == '1');
				}

			}
		}
		fileReader.Close();

		return 0;
	}

	template <size_t N, typename Graph, typename ArcFlagsMap>
	int save_arcflags(Graph& graph, ArcFlagsMap& arcflags, const char* PathToFile) {
		using namespace graph;
		using namespace graphUtil;
		using namespace std;

		FILE* outFile = fopen(PathToFile, "wt");
		if (outFile == nullptr) {
			std::cerr << "Can't open file " << PathToFile << endl;
			return 1;
		}

		for (const auto& v : Range(vertices(graph))) {
			for (const auto& edge : Range(out_edges(v, graph))) {
				auto bitset = get(arcflags, edge);
				fprintf(outFile, "%d %d ", source(edge, graph), target(edge, graph));
				for (const auto& bitIndex : Range(0, N)) {
					fprintf(outFile, "%c", bitset.GetBit(bitIndex) == true ? '1' : '0');
				}
				fprintf(outFile, "\n");
			}
		}
		fclose(outFile);

		return 0;
	}


	// uses dijkstra, therefore should have at least all property maps used by dijkstra
	template <size_t N, typename Graph, typename PredecessorMap, typename DistanceMap,
			  typename WeightMap, typename IndexMap, typename ColorMap, typename PartitionMap,
			  typename ArcFlagsMap>
	void arcflags_preprocess(Graph& graph, PredecessorMap& predecessor, DistanceMap& distance,
							 WeightMap& weight, IndexMap& index, ColorMap& color, PartitionMap& partition,
							 ArcFlagsMap& arcflags, double filter = 0) {
		auto invertedGraph = graph::ComplementGraph<Graph>(graph);
		//auto borderCnt = 0;
		for (const auto& v : graphUtil::Range(graph::vertices(invertedGraph))) {
			auto vPartIndex = get(partition, v);

			for (const auto& edge : graphUtil::Range(graph::out_edges(v, invertedGraph))) {
				const auto& to = target(edge, invertedGraph);
				if (get(partition, to) == vPartIndex) {
					auto& bitset = get(arcflags, edge);
					bitset.SetBit(vPartIndex);
				}
			}

			auto borderVertex = false;
			for (const auto& to : graphUtil::Range(graph::adjacent_vertices(v, invertedGraph))) {
				if (get(partition, to) != vPartIndex) {
					borderVertex = true;
					break;
				}
			}

			if (borderVertex) {
				//++borderCnt;
				graph::DefaultDijkstraVisitor<Graph> visitor;
				graph::dijkstra(invertedGraph, v, predecessor, distance, weight, index, color, visitor);

				for (const auto& fromVertex : graphUtil::Range(graph::vertices(invertedGraph))) {
					auto predVertex = get(predecessor, fromVertex);
					if (predVertex == fromVertex)
						continue;
					auto predecessorEdgeWeight = get(distance, fromVertex) - get(distance, predVertex);
					for (const auto& edge : graphUtil::Range(graph::in_edges(fromVertex, invertedGraph))) {
						auto toVertex = graph::source(edge, invertedGraph);
						if (toVertex == predVertex && get(weight, edge) == predecessorEdgeWeight) {
							//						if (get(distance, fromVertex) + get(weight, edge) == get(distance, toVertex)) {
							auto& bitset = get(arcflags, edge);
							bitset.SetBit(vPartIndex);
						}
					}
				}
			}
		}
		//printf("%d\n", borderCnt);
	};

	template <typename Graph, typename ArcFlagsMap, typename PartitionMap>
	struct ArcflagsQueryDijkstraVisitor : public graph::DefaultDijkstraVisitor<Graph> {
		ArcflagsQueryDijkstraVisitor(const ArcFlagsMap& arcflags, size_t targetPart)
			: arcflags(arcflags),
			  targetPart(targetPart) { }

		bool should_relax(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph) {
			auto to = target(edge, graph);
			auto& bitset = get(arcflags, edge);
			return bitset.GetBit(targetPart);
		}

	private:
		ArcFlagsMap arcflags;
		size_t targetPart;
	};

	template <size_t N, typename Graph, typename PredecessorMap, typename DistanceMap,
			  typename WeightMap, typename IndexMap, typename ColorMap, typename PartitionMap,
			  typename ArcFlagsMap>
	void arcflags_query(Graph& graph,
						const typename graph::graph_traits<Graph>::vertex_descriptor& s,
						const typename graph::graph_traits<Graph>::vertex_descriptor& t,
						PredecessorMap& predecessor, DistanceMap& distance,
						WeightMap& weight, IndexMap& index, ColorMap& color, PartitionMap& partition,
						ArcFlagsMap& arcflags) {
		auto visitor = ArcflagsQueryDijkstraVisitor<Graph, ArcFlagsMap, PartitionMap>(arcflags, get(partition, t));
		graph::dijkstra(graph, s, predecessor, distance, weight, index, color, visitor);
	};
};

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <graph/static_graph.hpp>
#include <graph/dijkstra.hpp>
#include <graph/bidirectional_dijkstra.hpp>
#include <arc-flags/Bitset.hpp>
#include <graph/io/FileReader.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/detail/ComplementGraph.hpp>

namespace arcflags
{
	template <typename PredecessorMapFTag, typename PredecessorMapBTag,
	          typename DistanceMapFTag, typename DistanceMapBTag,
	          typename WeightMapTag, typename IndexMapTag,
	          typename ColorMapFTag, typename ColorMapBTag,
	          typename ArcFlagsMapFTag, typename ArcFlagsMapBTag,
	          typename PartitionMapTag, size_t N,
	          typename BundledVertexProperties, typename BundledEdgeProperties>
	struct GenerateBiArcFlagsGraph
	{
	};

	template <typename PredecessorMapFTag, typename PredecessorMapBTag,
	          typename DistanceMapFTag, typename DistanceMapBTag,
	          typename WeightMapTag, typename IndexMapTag,
	          typename ColorMapFTag, typename ColorMapBTag,
	          typename ArcFlagsMapFTag, typename ArcFlagsMapBTag,
	          typename PartitionMapTag, size_t N,
	          typename... P1s, typename... P2s>
	struct GenerateBiArcFlagsGraph<PredecessorMapFTag, PredecessorMapBTag,
	                             DistanceMapFTag, DistanceMapBTag,
	                             WeightMapTag, IndexMapTag,
	                             ColorMapFTag, ColorMapBTag,
	                             ArcFlagsMapFTag, ArcFlagsMapBTag,
	                             PartitionMapTag, N,
	                             graph::Properties<P1s...>, graph::Properties<P2s...>>
	{
		using type = graph::StaticGraph<
			graph::Properties<
				graph::Property<PredecessorMapFTag,
				                graph::graph_traits<graph::StaticGraph<graph::Properties<>, graph::Properties<>>>::vertex_descriptor>,
				graph::Property<PredecessorMapBTag,
				                graph::graph_traits<graph::StaticGraph<graph::Properties<>, graph::Properties<>>>::vertex_descriptor>,
				graph::Property<DistanceMapFTag, uint32_t>,
				graph::Property<DistanceMapBTag, uint32_t>,
				graph::Property<ColorMapFTag, boost::two_bit_color_type>,
				graph::Property<ColorMapBTag, boost::two_bit_color_type>,
				graph::Property<PartitionMapTag, char>,
				P1s...>,
			graph::Properties<
				graph::Property<WeightMapTag, uint32_t>,
				graph::Property<ArcFlagsMapFTag, bitset::Bitset<N>>,
				graph::Property<ArcFlagsMapBTag, bitset::Bitset<N>>,
				P2s...>>;
	};

	template <size_t N, typename Graph, typename ArcFlagsMapF, typename ArcFlagsMapB>
	int read_bidirectional_arcflags(Graph& graph, ArcFlagsMapF& arcflagsF, ArcFlagsMapB& arcflagsB, const char* PathToFile)
	{
		using namespace graph;
		using namespace graphIO;
		using namespace graphUtil;
		using namespace std;

		FileReader fileReader;
		if (!fileReader.Open(PathToFile))
			return 1;

		for (const auto& v : Range(vertices(graph)))
		{
			for (const auto& outEdge : Range(out_edges(v, graph)))
			{
				assert(fileReader.HasNext());
				auto start = fileReader.NextUnsignedInt();
				auto end = fileReader.NextUnsignedInt();
				assert(start == source(outEdge, graph));
				assert(end == target(outEdge, graph));
				auto& arcFlagF = get(arcflagsF, outEdge);
				for (const auto& bitIndex : Range(0, N))
				{
					char bit = fileReader.NextChar();
					arcFlagF.SetBit(bitIndex, bit == '1');
				}
				auto& arcFlagB = get(arcflagsB, outEdge);
				for (const auto& bitIndex : Range(0, N))
				{
					char bit = fileReader.NextChar();
					arcFlagB.SetBit(bitIndex, bit == '1');
				}
			}
		}
		fileReader.Close();

		return 0;
	}

	template <size_t N, typename Graph, typename ArcFlagsMapF, typename ArcFlagsMapB>
	int save_bidirectional_arcflags(Graph& graph, ArcFlagsMapF& arcflagsF, ArcFlagsMapB& arcflagsB, const char* PathToFile)
	{
		using namespace graph;
		using namespace graphUtil;
		using namespace std;

		FILE* outFile = fopen(PathToFile, "wt");
		if (outFile == nullptr)
		{
			std::cerr << "Can't open file " << PathToFile << endl;
			return 1;
		}

		for (const auto& v : Range(vertices(graph)))
		{
			for (const auto& edge : Range(out_edges(v, graph)))
			{
				auto bitsetF = get(arcflagsF, edge);
				fprintf(outFile, "%d %d ", source(edge, graph), target(edge, graph));
				for (const auto& bitIndex : Range(0, N))
				{
					fprintf(outFile, "%c", bitsetF.GetBit(bitIndex) == true ? '1' : '0');
				}

				auto bitsetB = get(arcflagsB, edge);
				for (const auto& bitIndex : Range(0, N))
				{
					fprintf(outFile, "%c", bitsetB.GetBit(bitIndex) == true ? '1' : '0');
				}
				fprintf(outFile, "\n");
			}
		}
		fclose(outFile);

		return 0;
	}


	template <size_t N, typename Graph,
	          typename PredecessorMapF, typename PredecessorMapB,
	          typename DistanceMapF, typename DistanceMapB,
	          typename WeightMap, typename IndexMap,
	          typename ColorMapF, typename ColorMapB,
	          typename PartitionMap,
	          typename ArcFlagsMapF, typename ArcFlagsMapB>
	void arcflags_preprocess(Graph& graph,
	                         PredecessorMapF& predcessorF, PredecessorMapB& predcessorB,
	                         DistanceMapF& distanceF, DistanceMapB& distanceB,
	                         WeightMap& weight, IndexMap& index,
	                         ColorMapF& colorF, ColorMapB& colorB,
	                         PartitionMap& partition,
	                         ArcFlagsMapF& arcflagsF, ArcFlagsMapB& arcflagsB, double filter = 0.0)
	{
		arcflags_preprocess<N>(graph, predcessorF, distanceF, weight, index, colorF, partition, arcflagsF);
		auto invertedGraph = graph::ComplementGraph<Graph>(graph);
		arcflags_preprocess<N>(invertedGraph, predcessorB, distanceB, weight, index, colorB, partition, arcflagsB);
	}

	template <size_t N, typename Graph,
	          typename PredecessorMapF, typename PredecessorMapB,
	          typename DistanceMapF, typename DistanceMapB,
	          typename WeightMap, typename IndexMap,
	          typename ColorMapF, typename ColorMapB,
	          typename PartitionMap,
	          typename ArcFlagsMapF, typename ArcFlagsMapB>
	void arcflags_query(Graph& graph,
	                    const typename graph::graph_traits<Graph>::vertex_descriptor& s,
	                    const typename graph::graph_traits<Graph>::vertex_descriptor& t,
	                    PredecessorMapF& predcessorF, PredecessorMapB& predcessorB,
	                    DistanceMapF& distanceF, DistanceMapB& distanceB,
	                    WeightMap& weight, IndexMap& index,
	                    ColorMapF& colorF, ColorMapB& colorB,
	                    PartitionMap& partition,
	                    ArcFlagsMapF& arcflagsF, ArcFlagsMapB& arcflagsB)
	{
		auto visitorF = ArcflagsQueryDijkstraVisitor<Graph, ArcFlagsMapF, PartitionMap>(arcflagsF, get(partition, t));
		auto invertedFraph = graph::ComplementGraph<Graph>(graph);
		auto visitorB = ArcflagsQueryDijkstraVisitor<graph::ComplementGraph<Graph>, ArcFlagsMapB, PartitionMap>(arcflagsB, get(partition, s));
		graph::bidirectional_dijkstra(graph, s, t, predcessorF, predcessorB, distanceF, distanceB, weight, index, colorF, colorB, visitorF, visitorB);
	};
};


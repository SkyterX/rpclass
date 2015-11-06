#pragma once

#include <graph/static_graph.hpp>
#include <graph/dijkstra.hpp>
#include <arc-flags/Bitset.hpp>
#include <graph/io/FileReader.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/detail/ComplementGraph.hpp>


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
                typename graph::graph_traits<graph::StaticGraph<graph::Properties<>,graph::Properties<>>>::vertex_descriptor>,
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
int read_partitioning(Graph& graph, const char* PathToFile)
{
	graphIO::FileReader fileReader;
	fileReader.Open(PathToFile);
	auto partitionMap = get(PartitionMapTag(), graph);

	for (auto& v : graphUtil::Range(graph::vertices(graph)))
	{
		auto classIndex = fileReader.NextUnsignedInt();
		put(partitionMap, v, static_cast<char>(classIndex));
	}

	fileReader.Close();

	return 0;
};

template <typename Graph, typename ArcFlagsMap, typename PartitionMap>
struct ArcflagsBuildingDijkstraVisitor : public graph::DefaultDijkstraVisitor<Graph>
{
	ArcflagsBuildingDijkstraVisitor(size_t start_vertex_part_index, const ArcFlagsMap& arcflags)
		: startVertexPartIndex(start_vertex_part_index),
		  arcflags(arcflags)
	{
	}

	void examine_edge(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph)
	{
		auto& bitset = get(arcflags, edge);
		bitset.SetBit(startVertexPartIndex);
	};

private:
	size_t startVertexPartIndex;
	ArcFlagsMap arcflags;
};


// uses dijkstra, therefore should have at least all property maps used by dijkstra
template <size_t N, typename Graph, typename PredecessorMap, typename DistanceMap,
    typename WeightMap, typename IndexMap, typename ColorMap, typename PartitionMap, 
    typename ArcFlagsMap>
    void arcflags_preprocess(Graph& graph, PredecessorMap& predecessor, DistanceMap& distance,
        WeightMap& weight, IndexMap& index, ColorMap& color, PartitionMap& partition,
        ArcFlagsMap& arcflags, double filter = 0 )
{
	auto invertedGraph = graph::ComplementGraph<Graph>(graph);
	//auto borderCnt = 0;
	for (const auto& v : graphUtil::Range(graph::vertices(invertedGraph)))
	{
		auto vPartIndex = get(partition, v);
		
		for (const auto& edge : graphUtil::Range(graph::out_edges(v, invertedGraph)))
		{
			const auto& to = target(edge, invertedGraph);
			if (get(partition, to) == vPartIndex)
			{
				auto& bitset = get(arcflags, edge);
				bitset.SetBit(vPartIndex);
			}
		}
		
		auto borderVertex = false;
		for (const auto& to : graphUtil::Range(graph::adjacent_vertices(v, invertedGraph))) {
			if (get(partition, to) != vPartIndex)
			{
				borderVertex = true;
				break;
			}
		}

		if (borderVertex)
		{
			//++borderCnt;
			graph::dijkstra(invertedGraph,
				v,
				predecessor,
				distance,
				weight,
				index,
				color,
				ArcflagsBuildingDijkstraVisitor<graph::ComplementGraph<Graph>, ArcFlagsMap, PartitionMap>(get(partition, v), arcflags));
		}
	}
	//printf("%d\n", borderCnt);
};

template <typename Graph, typename ArcFlagsMap, typename PartitionMap>
struct ArcflagsQueryDijkstraVisitor : public graph::DefaultDijkstraVisitor<Graph>
{
	ArcflagsQueryDijkstraVisitor(const ArcFlagsMap& arcflags, size_t targetPart)
		: arcflags(arcflags),
		  targetPart(targetPart)
	{
	}

	bool should_relax(const typename graph::graph_traits<Graph>::edge_descriptor& edge, Graph& graph)
	{
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
    graph::dijkstra(graph, s, predecessor, distance, weight, index, color, ArcflagsQueryDijkstraVisitor<Graph, ArcFlagsMap, PartitionMap>(arcflags, get(partition, t)));
};
};
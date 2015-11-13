#pragma once
#include <cstdio>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>
#include <graph/static_graph.hpp>
#include <graph/dijkstra.hpp>
#include <arc-flags/Bitset.hpp>
#include <graph/io/FileReader.hpp>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <graph/detail/ComplementGraph.hpp>

namespace arcflags
{
	template <typename ArcFlag, size_t N>
	struct ArcFlagComparer {
		size_t CountBits(const ArcFlag& x) const {
			size_t bits = 0;
			for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
				if (x.GetBit(bitIndex))
					++bits;
			}
			return bits;
		}

		bool LexicographicalCompare(const ArcFlag& x, const ArcFlag& y) const {
			for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
				if (x.GetBit(bitIndex) != y.GetBit(bitIndex))
					return x.GetBit(bitIndex);
			}
			return false;
		}

		bool operator()(const ArcFlag& x, const ArcFlag& y) const {
			auto xBits = CountBits(x);
			auto yBits = CountBits(y);
			return xBits < yBits || (xBits == yBits && LexicographicalCompare(x, y));
		}
	};

	template <typename ArcFlag, size_t N>
	ArcFlag FullArcFlag() {
		ArcFlag arcFlag;
		for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
			arcFlag.SetBit(bitIndex, true);
		}
		return arcFlag;
	}

	template <size_t N, typename ArcFlag, typename ArcFlagMap>
	void FindClosestArcFlag(const ArcFlag& startArcFlag, ArcFlagMap& arcFlagMap) {
		using namespace std;
		using namespace graph;
		using namespace graphUtil;

		std::queue<ArcFlag> q;
		q.push(startArcFlag);
		while (!q.empty()) {
			auto arcFlag = q.front();
			q.pop();
			for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
				if (arcFlag.GetBit(bitIndex) == true) continue;
				arcFlag.SetBit(bitIndex, true);
				auto findIt = arcFlagMap.find(arcFlag);
				if (findIt != arcFlagMap.end()) {
					arcFlagMap[startArcFlag] = findIt->second;
					return;
				}
				q.push(arcFlag);
				arcFlag.SetBit(bitIndex, false);
			}
		}
	}

	template <size_t N, typename Graph, typename ArcFlagsMap>
	void arcflags_reduce_greedy(Graph& graph, ArcFlagsMap& arcflags, double filter = 0) {
		using namespace std;
		using namespace graph;
		using namespace graphUtil;

		using ArcFlagType = typename ArcFlagsMap::value_type;
		using ArcFlagComparerType = ArcFlagComparer<ArcFlagType, N>;

		map<ArcFlagType, uint32_t, ArcFlagComparerType> arcFlagCount;

		for (const auto& v : Range(vertices(graph))) {
			for (const auto& e : Range(out_edges(v, graph))) {
				++arcFlagCount[get(arcflags, e)];
			}
		}

		vector<pair<uint32_t, ArcFlagType>> countedArcFlags;
		countedArcFlags.reserve(arcFlagCount.size());
		for (auto& kv : arcFlagCount) {
			countedArcFlags.push_back(make_pair(kv.second, kv.first));
		}
		auto arcFlagComparer = ArcFlagComparerType();
		stable_sort(countedArcFlags.begin(), countedArcFlags.end(),
		            [&arcFlagComparer](const pair<uint32_t, ArcFlagType>& a, const pair<uint32_t, ArcFlagType>& b) {
			            return a.first < b.first || (a.first == b.first && arcFlagComparer(a.second, b.second));
		            });
		auto targetSize = static_cast<size_t>(ceil(countedArcFlags.size() * (1.0 - filter)));
		countedArcFlags.resize(targetSize);

		auto fullArcFlag = FullArcFlag<ArcFlagType, N>();
		map<ArcFlagType, ArcFlagType, ArcFlagComparerType> arcFlagMap;
		arcFlagMap[fullArcFlag] = fullArcFlag;
		for (auto& pair : countedArcFlags) {
			arcFlagMap[pair.second] = pair.second;
		}
		for (auto& kv : Range(arcFlagCount.rbegin(), arcFlagCount.rend())) {
			auto arcFlag = kv.first;
			if (arcFlagMap.find(arcFlag) != arcFlagMap.end())
				continue;
			FindClosestArcFlag<N>(arcFlag, arcFlagMap);
		}

		for (const auto& v : Range(vertices(graph))) {
			for (const auto& e : Range(out_edges(v, graph))) {
				auto arcFlag = get(arcflags, e);
				auto newArcFlag = arcFlagMap[arcFlag];
				put(arcflags, e, newArcFlag);
			}
		}
	}

	template <size_t N, typename ArcFlag, typename ArcFlagCountMap>
	void PropagateRank(const ArcFlag& startArcFlag, ArcFlagCountMap& arcFlagCount) {
		using namespace std;
		using namespace graph;
		using namespace graphUtil;

		const double fadeAlpha = 0.5;

		std::queue<pair<ArcFlag, uint8_t>> q;
		q.push(make_pair(startArcFlag, 0));
		double startFlagCount = arcFlagCount[startArcFlag];
		int breakDistance = 3;
		while (!q.empty()) {
			ArcFlag arcFlag;
			uint8_t distance;
			std::tie(arcFlag, distance) = q.front();
			q.pop();
			for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
				if (arcFlag.GetBit(bitIndex) == true) continue;
				arcFlag.SetBit(bitIndex, true);
				arcFlagCount[arcFlag] += startFlagCount * pow(fadeAlpha, distance + 1);
				if(distance < breakDistance)
					q.push(make_pair(arcFlag, distance + 1));
				arcFlag.SetBit(bitIndex, false);
			}
		}
	}

	template <size_t N, typename Graph, typename ArcFlagsMap>
	void arcflags_reduce_ranked(Graph& graph, ArcFlagsMap& arcflags, double filter = 0) {
		using namespace std;
		using namespace graph;
		using namespace graphUtil;

		using ArcFlagType = typename ArcFlagsMap::value_type;
		using ArcFlagComparerType = ArcFlagComparer<ArcFlagType, N>;

		map<ArcFlagType, double, ArcFlagComparerType> arcFlagCount;

		for (const auto& v : Range(vertices(graph))) {
			for (const auto& e : Range(out_edges(v, graph))) {
				++arcFlagCount[get(arcflags, e)];
			}
		}
		auto targetSize = static_cast<size_t>(ceil(arcFlagCount.size() * (1.0 - filter)));

		vector<pair<double, ArcFlagType>> countedArcFlags;
		countedArcFlags.reserve(arcFlagCount.size());
		for (auto& kv : arcFlagCount) {
			countedArcFlags.push_back(make_pair(kv.second, kv.first));
		}

		for(auto& kv : countedArcFlags) {
			PropagateRank<N>(kv.second, arcFlagCount);
		}

		countedArcFlags.clear();
		countedArcFlags.reserve(arcFlagCount.size());
		for (auto& kv : arcFlagCount) {
			countedArcFlags.push_back(make_pair(kv.second, kv.first));
		}
		auto arcFlagComparer = ArcFlagComparerType();
		stable_sort(countedArcFlags.begin(), countedArcFlags.end(),
			[&arcFlagComparer](const pair<double, ArcFlagType>& a, const pair<double, ArcFlagType>& b) {
			return a.first < b.first || (a.first == b.first && arcFlagComparer(a.second, b.second));
		});
		countedArcFlags.resize(targetSize);

		auto fullArcFlag = FullArcFlag<ArcFlagType, N>();
		map<ArcFlagType, ArcFlagType, ArcFlagComparerType> arcFlagMap;
		arcFlagMap[fullArcFlag] = fullArcFlag;
		for (auto& pair : countedArcFlags) {
			arcFlagMap[pair.second] = pair.second;
		}
		for (auto& kv : Range(arcFlagCount.rbegin(), arcFlagCount.rend())) {
			auto arcFlag = kv.first;
			if (arcFlagMap.find(arcFlag) != arcFlagMap.end())
				continue;
			FindClosestArcFlag<N>(arcFlag, arcFlagMap);
		}

		for (const auto& v : Range(vertices(graph))) {
			for (const auto& e : Range(out_edges(v, graph))) {
				auto arcFlag = get(arcflags, e);
				auto newArcFlag = arcFlagMap[arcFlag];
				put(arcflags, e, newArcFlag);
			}
		}
	}
}

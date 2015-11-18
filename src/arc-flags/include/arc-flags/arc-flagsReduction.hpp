#pragma once
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <graph/static_graph.hpp>
#include <graph/properties.hpp>
#include <graph/detail/ComplementGraph.hpp>

namespace arcflags
{
	template <size_t N, typename ArcFlag>
	size_t CountBits(const ArcFlag& x) {
		size_t bits = 0;
		for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
			if (x.GetBit(bitIndex))
				++bits;
		}
		return bits;
	}

	template <typename ArcFlag, size_t N>
	struct ArcFlagComparer {
		bool LexicographicalCompare(const ArcFlag& x, const ArcFlag& y) const {
			for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
				if (x.GetBit(bitIndex) != y.GetBit(bitIndex))
					return x.GetBit(bitIndex);
			}
			return false;
		}

		bool operator()(const ArcFlag& x, const ArcFlag& y) const {
			auto xBits = CountBits<N>(x);
			auto yBits = CountBits<N>(y);
			return xBits < yBits || (xBits == yBits && LexicographicalCompare(x, y));
		}
	};

	template <typename ArcFlag, size_t N>
	struct ArcFlagStorage {
		std::unordered_set<ArcFlag> ArcFlags;
		std::vector<ArcFlag> ArcFlagsByBits[N + 1];

		void Add(const ArcFlag& arcFlag) {
			if (ArcFlags.insert(arcFlag).second)
				ArcFlagsByBits[CountBits<N>(arcFlag)].push_back(arcFlag);
		}

		const ArcFlag& FindClosest(const ArcFlag& arcFlag) {
			if (ArcFlags.find(arcFlag) != ArcFlags.end())
				return arcFlag;
			for (int bits = CountBits<N>(arcFlag); bits < N; ++bits) {
				for (const auto& storedArcFlag : ArcFlagsByBits[bits]) {
					bool isValid = true;
					for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
						if (arcFlag.GetBit(bitIndex) && !storedArcFlag.GetBit(bitIndex)) {
							isValid = false;
							break;
						}
					}
					if (isValid) {
						return storedArcFlag;
					}
				}
			}
			assert(false);
			return arcFlag;
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
	void FindClosestArcFlag(const ArcFlag& startArcFlag, ArcFlagMap& arcFlagMap, ArcFlagStorage<ArcFlag, N>& selectedArcFlags) {
		using namespace std;
		using namespace graph;
		using namespace graphUtil;
		const int maxDistance = 1;

		std::queue<pair<ArcFlag, int>> q;
		q.push(make_pair(startArcFlag, 0));
		int iterationId = 0;
		while (!q.empty() && iterationId < selectedArcFlags.ArcFlags.size()) {
			++iterationId;
			ArcFlag arcFlag;
			int distance;
			std::tie(arcFlag, distance) = q.front();
			q.pop();
			for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
				if (arcFlag.GetBit(bitIndex) == true) continue;
				arcFlag.SetBit(bitIndex, true);
				auto findIt = arcFlagMap.find(arcFlag);
				if (findIt != arcFlagMap.end()) {
					arcFlagMap[startArcFlag] = findIt->second;
					return;
				}
				if (distance < maxDistance)
					q.push(make_pair(arcFlag, distance + 1));
				arcFlag.SetBit(bitIndex, false);
			}
		}

		arcFlagMap[startArcFlag] = selectedArcFlags.FindClosest(startArcFlag);
	}

	template <size_t N, typename Graph, typename ArcFlagsMap>
	void arcflags_reduce_greedy(Graph& graph, ArcFlagsMap& arcflags, double filter = 0) {
		using namespace std;
		using namespace graph;
		using namespace graphUtil;

		using ArcFlagType = typename ArcFlagsMap::value_type;
		using ArcFlagComparerType = ArcFlagComparer<ArcFlagType, N>;

		unordered_map<ArcFlagType, uint32_t> arcFlagCount;
		cout << "Counting arc-flags" << endl;
		for (const auto& v : Range(vertices(graph))) {
			for (const auto& e : Range(out_edges(v, graph))) {
				++arcFlagCount[get(arcflags, e)];
			}
		}

		cout << "Total : " << arcFlagCount.size() << endl;
		cout << "Sorting arc-flags" << endl;
		vector<pair<uint32_t, ArcFlagType>> countedArcFlags;
		vector<ArcFlagType> allArcFlags;
		countedArcFlags.reserve(arcFlagCount.size());
		allArcFlags.reserve(arcFlagCount.size());
		for (auto& kv : arcFlagCount) {
			countedArcFlags.push_back(make_pair(kv.second, kv.first));
			allArcFlags.push_back(kv.first);
		}
		auto arcFlagComparer = ArcFlagComparerType();
		stable_sort(countedArcFlags.begin(), countedArcFlags.end(),
		            [&arcFlagComparer](const pair<uint32_t, ArcFlagType>& a, const pair<uint32_t, ArcFlagType>& b) {
			            return a.first < b.first || (a.first == b.first && arcFlagComparer(a.second, b.second));
		            });
		stable_sort(allArcFlags.begin(), allArcFlags.end(), ArcFlagComparerType());
		reverse(allArcFlags.begin(), allArcFlags.end());
		auto targetSize = static_cast<size_t>(ceil(countedArcFlags.size() * (1.0 - filter)));
		countedArcFlags.resize(targetSize);
		cout << "Arc-flags left : " << countedArcFlags.size() << endl;
		cout << "Creating mapping" << endl;

		auto fullArcFlag = FullArcFlag<ArcFlagType, N>();
		unordered_map<ArcFlagType, ArcFlagType> arcFlagMap;
		ArcFlagStorage<ArcFlagType, N> selectedArcFlags;
		arcFlagMap[fullArcFlag] = fullArcFlag;
		selectedArcFlags.Add(fullArcFlag);
		for (auto& pair : countedArcFlags) {
			arcFlagMap[pair.second] = pair.second;
			selectedArcFlags.Add(pair.second);
		}
		for (auto& arcFlag : allArcFlags) {
			if (arcFlagMap.find(arcFlag) != arcFlagMap.end())
				continue;
			FindClosestArcFlag<N>(arcFlag, arcFlagMap, selectedArcFlags);
		}

		cout << "Mapping arc-flags" << endl;
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
		int breakDistance = 0;
		while (!q.empty()) {
			ArcFlag arcFlag;
			uint8_t distance;
			std::tie(arcFlag, distance) = q.front();
			q.pop();
			for (size_t bitIndex = 0; bitIndex < N; ++bitIndex) {
				if (arcFlag.GetBit(bitIndex) == true) continue;
				arcFlag.SetBit(bitIndex, true);
				arcFlagCount[arcFlag] += startFlagCount * pow(fadeAlpha, distance + 1);
				if (distance < breakDistance)
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

		unordered_map<ArcFlagType, uint32_t> arcFlagCount;
		cout << "Counting arc-flags" << endl;
		for (const auto& v : Range(vertices(graph))) {
			for (const auto& e : Range(out_edges(v, graph))) {
				++arcFlagCount[get(arcflags, e)];
			}
		}

		cout << "Total : " << arcFlagCount.size() << endl;
		cout << "Propagating ranks" << endl;

		auto targetSize = static_cast<size_t>(ceil(arcFlagCount.size() * (1.0 - filter)));

		vector<pair<uint32_t, ArcFlagType>> countedArcFlags;
		vector<ArcFlagType> allArcFlags;
		countedArcFlags.reserve(arcFlagCount.size());
		allArcFlags.reserve(arcFlagCount.size());
		for (auto& kv : arcFlagCount) {
			countedArcFlags.push_back(make_pair(kv.second, kv.first));
			allArcFlags.push_back(kv.first);
		}
		stable_sort(allArcFlags.begin(), allArcFlags.end(), ArcFlagComparerType());
		reverse(allArcFlags.begin(), allArcFlags.end());

		for (auto& kv : countedArcFlags) {
			PropagateRank<N>(kv.second, arcFlagCount);
		}

		cout << "Sorting arc-flags" << endl;

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
		cout << "Arc-flags left : " << targetSize << endl;
		cout << "Creating mapping" << endl;

		auto fullArcFlag = FullArcFlag<ArcFlagType, N>();
		unordered_map<ArcFlagType, ArcFlagType> arcFlagMap;
		ArcFlagStorage<ArcFlagType, N> selectedArcFlags;
		arcFlagMap[fullArcFlag] = fullArcFlag;
		selectedArcFlags.Add(fullArcFlag);
		for (auto& pair : countedArcFlags) {
			arcFlagMap[pair.second] = pair.second;
			selectedArcFlags.Add(pair.second);
		}
		for (auto& arcFlag : allArcFlags) {
			if (arcFlagMap.find(arcFlag) != arcFlagMap.end())
				continue;
			FindClosestArcFlag<N>(arcFlag, arcFlagMap, selectedArcFlags);
		}

		cout << "Mapping arc-flags" << endl;
		for (const auto& v : Range(vertices(graph))) {
			for (const auto& e : Range(out_edges(v, graph))) {
				auto arcFlag = get(arcflags, e);
				auto newArcFlag = arcFlagMap[arcFlag];
				put(arcflags, e, newArcFlag);
			}
		}
	}
}

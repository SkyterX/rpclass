#include <gtest/gtest.h>
#include <graph/queue/SetQueue.hpp>
#include <graph/queue/HeapQueue.hpp>
#include <graph/queue/FibonacciHeapQueue.hpp>
#include <graph/queue/D-aryHeapQueue.hpp>
#include <graph/queue/SegmentTreeQueue.hpp>
#include <graph/detail/util/Collection.hpp>
#include <random>
#include <type_traits>
#include <vector>
#include <algorithm>

using namespace std;
using namespace graph::queue;
using namespace graphUtil;

const bool Verify = true;

const int TestItemsCount = 10000;
const int DecreaseQueriesCount = 10 * TestItemsCount;
const int RandomQueriesCount = 100 * TestItemsCount;
const int RandomSeed = 231274120;
const int MaxKey = 1e9;
const int MinStartKey = 1e6;
const int MinDecreasedKey = 0;
const double DeleteMinProbability = 0.2;

uniform_int_distribution<size_t> ItemIdDistribution(0, TestItemsCount - 1);
uniform_real_distribution<double> Probability(0.0, 1.0);

using KeyType = uint32_t;
using DataType = uint32_t;
using KeyDistribution = uniform_int_distribution<KeyType>;

template <typename TQueue>
void Queue_SequentialTest(TQueue& queue) {
	mt19937 generator(RandomSeed);
	KeyDistribution startKeyDistribution(MinStartKey, MaxKey);

	vector<pair<KeyType, DataType>> items;
	for (auto x : Range(0, TestItemsCount)) {
		KeyType key = startKeyDistribution(generator);
		items.push_back(make_pair(key, x));
		queue.Insert(key, x);
	}

	for (auto iterationId : Range(0, DecreaseQueriesCount)) {
		auto itemId = ItemIdDistribution(generator);
		auto item = items[itemId];
		if (item.first == 0)
			continue;
		KeyDistribution newKeyDistribution(MinDecreasedKey, item.first - 1);
		KeyType newKey = newKeyDistribution(generator);
		queue.DecreaseKey(item.first, item.second, newKey);
		items[itemId].first = newKey;
	}

	sort(items.begin(), items.end());

	for (auto& item : items) {
		EXPECT_EQ(item.first, queue.PeekMin().Key());
		EXPECT_FALSE(queue.IsEmpty());
		queue.DeleteMin();
	}
	EXPECT_TRUE(queue.IsEmpty());
}

template <typename TQueue>
void Queue_RandomTest(TQueue& queue) {
	mt19937 generator(RandomSeed);
	KeyDistribution startKeyDistribution(MinStartKey, MaxKey);
	vector<pair<KeyType, DataType>> items(TestItemsCount);
	vector<bool> isEnqueued(TestItemsCount);
	set<QueueItem<KeyType, DataType>> referenceQueue;

	for (auto x : Range(0, TestItemsCount)) {
		KeyType key = startKeyDistribution(generator);
		items[x] = make_pair(key, x);
		isEnqueued[x] = false;
	}

	for (auto iterationId : Range(0, RandomQueriesCount)) {
		if (Verify) {
			EXPECT_EQ(queue.IsEmpty(), referenceQueue.empty());
			if (!referenceQueue.empty())
				EXPECT_EQ(referenceQueue.begin()->Key(), queue.PeekMin().Key());
		}
		if (!queue.IsEmpty() && Probability(generator) < DeleteMinProbability) {
			auto minItem = queue.PeekMin();
			if (Verify) {
				EXPECT_TRUE(referenceQueue.find(minItem) != referenceQueue.end());
				referenceQueue.erase(minItem);
			}
			isEnqueued[minItem.Data()] = false;
			queue.DeleteMin();
			continue;
		}
		auto itemId = ItemIdDistribution(generator);
		auto& item = items[itemId];
		if (!isEnqueued[itemId]) {
			item.first = startKeyDistribution(generator);

			if (Verify) {
				auto queueItem = MakeQueueItem(item.first, item.second);
				EXPECT_FALSE(referenceQueue.find(queueItem) != referenceQueue.end());
				referenceQueue.insert(queueItem);
			}

			queue.Insert(item.first, item.second);
			isEnqueued[itemId] = true;
		}
		else {
			if (item.first == 0)
				continue;
			KeyDistribution newKeyDistribution(MinDecreasedKey, item.first - 1);
			KeyType newKey = newKeyDistribution(generator);

			if (Verify) {
				auto queueItem = MakeQueueItem(item.first, item.second);
				auto findIt = referenceQueue.find(queueItem);
				EXPECT_TRUE(findIt != referenceQueue.end());
				referenceQueue.erase(findIt);
				queueItem = MakeQueueItem(newKey, item.second);
				referenceQueue.insert(queueItem);
			}

			queue.DecreaseKey(item.first, item.second, newKey);
			item.first = newKey;
		}
	}
}

TEST(PriorityQueue, Set_Sequential) {
	auto queue = SetQueue<KeyType, DataType>();
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, Set_Random) {
	auto queue = SetQueue<KeyType, DataType>();
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, Heap_Sequential) {
	auto queue = HeapQueue<KeyType, DataType>(TestItemsCount);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, Heap_Random) {
	auto queue = HeapQueue<KeyType, DataType>(TestItemsCount);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, Fibonacci_Sequential) {
	auto queue = FibonacciHeapQueue<KeyType, DataType>(TestItemsCount);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, Fibonacci_Random) {
	auto queue = FibonacciHeapQueue<KeyType, DataType>(TestItemsCount);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, SegmentTree_Sequential) {
	auto queue = SegmentTreeQueue<KeyType, DataType>(TestItemsCount);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, SegmentTree_Random) {
	auto queue = SegmentTreeQueue<KeyType, DataType>(TestItemsCount);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, NarySegmentTree_3_Sequential) {
	auto queue = NarySegmentTreeQueue<KeyType, DataType, 3>(TestItemsCount);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, NarySegmentTree_3_Random) {
	auto queue = NarySegmentTreeQueue<KeyType, DataType, 3>(TestItemsCount);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, NarySegmentTree_4_Sequential) {
	auto queue = NarySegmentTreeQueue<KeyType, DataType, 4>(TestItemsCount);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, NarySegmentTree_4_Random) {
	auto queue = NarySegmentTreeQueue<KeyType, DataType, 4>(TestItemsCount);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, DHeap_2_Sequential) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 2);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, DHeap_2_Random) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 2);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, DHeap_3_Sequential) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 3);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, DHeap_3_Random) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 3);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, DHeap_4_Sequential) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 4);
	Queue_SequentialTest(queue);
}

TEST(PriorityQueue, DHeap_4_Random) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 4);
	Queue_RandomTest(queue);
}

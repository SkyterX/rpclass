#include <gtest/gtest.h>
#include <graph/queue/SetQueue.hpp>
#include <graph/queue/HeapQueue.hpp>
#include <graph/queue/FibonacciHeapQueue.hpp>
#include <graph/queue/D-aryHeapQueue.hpp>
#include <graph/detail/util/Collection.hpp>
#include <random>
#include <type_traits>
#include <vector>
#include <algorithm>

using namespace std;
using namespace graph::queue;
using namespace graphUtil;

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

	for (auto x : Range(0, TestItemsCount)) {
		KeyType key = startKeyDistribution(generator);
		items.push_back(make_pair(key, x));
		isEnqueued.push_back(false);
	}

	for (auto iterationId : Range(0, RandomQueriesCount)) {
		if (!queue.IsEmpty() && Probability(generator) < DeleteMinProbability) {
			isEnqueued[queue.PeekMin().Data()] = false;
			queue.DeleteMin();
			continue;
		}
		auto itemId = ItemIdDistribution(generator);
		auto& item = items[itemId];
		if (!isEnqueued[item.second]) {
			item.first = startKeyDistribution(generator);
			queue.Insert(item.first, item.second);
			isEnqueued[item.second] = true;
		}
		else {
			if (item.first == 0)
				continue;
			KeyDistribution newKeyDistribution(MinDecreasedKey, item.first - 1);
			KeyType newKey = newKeyDistribution(generator);
			queue.DecreaseKey(item.first, item.second, newKey);
			item.first = newKey;
		}
	}

	sort(items.begin(), items.end());
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

TEST(PriorityQueue, DHeap_2_Random) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 2);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, DHeap_3_Random) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 3);
	Queue_RandomTest(queue);
}

TEST(PriorityQueue, DHeap_4_Random) {
	auto queue = DHeapQueue<KeyType, DataType>(TestItemsCount, 4);
	Queue_RandomTest(queue);
}
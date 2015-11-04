#include <gtest/gtest.h>
#include <graph/queue/SetQueue.hpp>
#include <graph/queue/HeapQueue.hpp>
#include <graph/detail/util/Collection.hpp>
#include <random>
#include <type_traits>
#include <vector>
#include <algorithm>

using namespace std;
using namespace graph::queue;
using namespace graphUtil;

const int TestItemsCount = 100000;
const int TestDecreaseCount = 10 * TestItemsCount;
const int RandomSeed = 231274120;
const int MaxKey = 1e9;
const int MinStartKey = 1e6;
const int MinDecreasedKey = 0;

uniform_int_distribution<size_t> ItemIdDistribution(0, TestItemsCount-1);

using KeyType = uint32_t;
using DataType = uint32_t;
using KeyDistribution = uniform_int_distribution<KeyType>;

template<typename TQueue>
void TestQueue(TQueue& queue) {
	mt19937 generator(RandomSeed);
	KeyDistribution startKeyDistribution(MinStartKey, MaxKey);

	vector<pair<KeyType, DataType>> items;
	for (auto x : Range(0, TestItemsCount)) {
		KeyType key = startKeyDistribution(generator);
		items.push_back(make_pair(key,x));
		queue.Insert(key, x);
	}

	for(auto iterationId : Range(0, TestDecreaseCount)) {
		auto itemId = ItemIdDistribution(generator);
		auto item = items[itemId];
		if (item.first == 0) 
			continue;
		KeyDistribution newKeyDistribution(MinDecreasedKey, item.first-1);
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

TEST(PriorityQueue, Set) {
	auto queue = SetQueue<KeyType, DataType>();
	TestQueue(queue);
}

TEST(PriorityQueue, Heap) {
	auto queue = HeapQueue<KeyType, DataType>(TestItemsCount);
	TestQueue(queue);
}

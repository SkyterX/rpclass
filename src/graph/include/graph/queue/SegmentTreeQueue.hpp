#pragma once
#include <graph/queue/PriorityQueue.hpp>
#include <vector>

namespace graph
{
	namespace queue
	{
		template <typename TKey, typename TDataId>
		class SegmentTreeQueue {
		private:
			using QueueItemType = QueueItem<TKey, TDataId>;

			std::vector<QueueItemType> tree;
			TDataId baseSize;

			void InsertInternal(TDataId index, QueueItemType item) {
				index += baseSize;
				tree[index] = item;
				do {
					index >>= 1;
					auto& left = tree[2 * index];
					auto& right = tree[2 * index + 1];
					auto aggregate = left < right ? left : right;
					if (tree[index] == aggregate)
						break;
					tree[index] = aggregate;
				}
				while (index > 1);
			}

			static TKey InfinityKey() {
				return std::numeric_limits<TKey>::max();
			}

		public:

			SegmentTreeQueue(TDataId dataIdSize) {
				baseSize = dataIdSize + 1;
				tree.resize(2 * baseSize, QueueItemType(InfinityKey(), 0));
			}

			void Insert(TKey key, TDataId dataId) {
				InsertInternal(dataId, MakeQueueItem(key, dataId));
			}

			const QueueItem<TKey, TDataId>& PeekMin() const {
				return tree[1];
			}

			void DeleteMin() {
				Insert(InfinityKey(), tree[1].Data());
			}

			void DecreaseKey(const TKey& key, const TDataId& dataId, const TKey& newKey) {
				Insert(newKey, dataId);
			}

			bool IsEmpty() const {
				return tree[1].Key() == InfinityKey();
			}
		};


		template <typename TKey, typename TDataId, int N = 2>
		class NarySegmentTreeQueue {
		private:
			using QueueItemType = QueueItem<TKey, TDataId>;

			std::vector<QueueItemType> tree;
			TDataId baseSize;

			void InsertInternal(TDataId index, QueueItemType item) {
				index += baseSize;
				tree[index] = item;
				do {
					index = (index - 1) / N;
					TDataId firstChildIndex = N * index + 1;
					auto aggregate = tree[firstChildIndex];
					for (int i = 1; i < N; ++i) {
						if (tree[firstChildIndex + i] < aggregate)
							aggregate = tree[firstChildIndex + i];
					}
					if (tree[index] == aggregate)
						break;
					tree[index] = aggregate;
				}
				while (index > 0);
			}

			static TKey InfinityKey() {
				return std::numeric_limits<TKey>::max();
			}

		public:

			NarySegmentTreeQueue(TDataId dataIdSize) {
				int treeSize = 1;
				baseSize = 0;
				while (treeSize < dataIdSize) {
					baseSize += treeSize;
					treeSize *= N;
				}
				tree.resize(treeSize + baseSize, QueueItemType(InfinityKey(), 0));
			}

			void Insert(TKey key, TDataId dataId) {
				InsertInternal(dataId, MakeQueueItem(key, dataId));
			}

			const QueueItem<TKey, TDataId>& PeekMin() const {
				return tree[0];
			}

			void DeleteMin() {
				Insert(InfinityKey(), tree[0].Data());
			}

			void DecreaseKey(const TKey& key, const TDataId& dataId, const TKey& newKey) {
				Insert(newKey, dataId);
			}

			bool IsEmpty() const {
				return tree[0].Key() == InfinityKey();
			}
		};
	}
}

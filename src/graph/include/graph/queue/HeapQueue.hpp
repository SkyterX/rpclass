#pragma once
#include <graph/queue/PriorityQueue.hpp>
#include <queue>
#include <cassert>
#include <functional>
#include <vector>

namespace graph {
	namespace queue {
		template <typename TKey, typename TDataId>
		class HeapQueue {
		private:
			using QueueItemType = QueueItem<TKey, TDataId>;

			std::priority_queue<QueueItemType, std::vector<QueueItemType>, std::greater<QueueItemType>> q;
			std::vector<TKey> keys;
			std::vector<bool> isDeleted;

			void SkipDeleted() {
				while (!q.empty() && (isDeleted[q.top().Data()] || q.top().Key() != keys[q.top().Data()]))
					q.pop();
			}
		public:

			HeapQueue(int dataIdSize) {
				Resize(dataIdSize);
			}

			void Resize(int dataIdSize) {
				isDeleted.resize(dataIdSize, false);
				keys.resize(dataIdSize);
			}

			void Insert(TKey key, TDataId dataId) {
				isDeleted[dataId] = false;
				keys[dataId] = key;
				q.push(MakeQueueItem(key, dataId));
			}

			const QueueItem<TKey, TDataId>& PeekMin() const {
				assert(!this->IsEmpty());
				return q.top();
			}

			void DeleteMin() {
				assert(!this->IsEmpty());
				isDeleted[q.top().Data()] = true;
				SkipDeleted();
			}

			void DecreaseKey(const TKey& key, const TDataId& dataId, const TKey& newKey) {
				keys[dataId] = newKey;
				Insert(newKey, dataId);
			}

			bool IsEmpty() const {
				return q.empty();
			}
		};


		// Items cannot be reinserted.
		template <typename TKey, typename TDataId>
		class FastHeapQueue {
		private:
			using QueueItemType = QueueItem<TKey, TDataId>;
			using QueueType = std::priority_queue<QueueItemType, std::vector<QueueItemType>, std::greater<QueueItemType>>;
			using IterationType = uint32_t;

			QueueType q;
			std::vector<IterationType> isDeleted;
			IterationType iterationId;

			void SkipDeleted() {
				while (!q.empty() && isDeleted[q.top().Data()] == iterationId)
					q.pop();
			}
		public:

			FastHeapQueue(int dataIdSize = 0) {
				iterationId = 1;
				Resize(dataIdSize);
			}
			
			void Resize(int dataIdSize) {
				isDeleted.resize(dataIdSize, 0);
			}

			void Clear() {
				++iterationId;
				q = QueueType();
			}

			void Insert(TKey key, TDataId dataId) {
				assert(isDeleted[dataId] < iterationId);
				q.push(MakeQueueItem(key, dataId));
			}

			const QueueItem<TKey, TDataId>& PeekMin() const {
				assert(!this->IsEmpty());
				return q.top();
			}

			void DeleteMin() {
				assert(!this->IsEmpty());
				isDeleted[q.top().Data()] = iterationId;
				SkipDeleted();
			}

			void DecreaseKey(const TKey& key, const TDataId& dataId, const TKey& newKey) {
				Insert(newKey, dataId);
			}

			bool IsEmpty() const {
				return q.empty();
			}
		};
	}
}

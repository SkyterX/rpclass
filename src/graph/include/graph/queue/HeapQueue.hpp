#pragma once
#include <graph/queue/PriorityQueue.hpp>
#include <queue>
#include <cassert>

namespace graph {
	namespace queue {
		template <typename TKey, typename TDataId>
		class HeapQueue {
		private:
			std::priority_queue<QueueItem<TKey, TDataId>> q;
			std::vector<bool> isDeleted;

			void SkipDeleted() {
				while (!q.empty() && isDeleted[q.top().Data()])
					q.pop();
			}
		public:

			HeapQueue(int dataIdSize) {
				isDeleted.resize(dataIdSize, false);
			}

			void Insert(TKey key, TDataId dataId) {
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
				Insert(newKey, dataId);
			}

			bool IsEmpty() const {
				return q.empty();
			}
		};
	}
}

#pragma once
#include <set>
#include <queue>
#include <utility>
#include <graph/queue/PriorityQueue.hpp>
#include <cassert>

namespace graph {
	namespace queue {
		template <typename TKey, typename TData>
		class SetQueue {
		private:
			std::set<QueueItem<TKey, TData>> q;
		public:

			const QueueItem<TKey, TData>& PeekMin() const {
				assert(!this->IsEmpty());
				return *q.begin();
			}

			void DeleteMin() {
				assert(!this->IsEmpty());
				q.erase(q.begin());
			}

			void Insert(const TKey& key, const TData& data) {
				q.insert(MakeQueueItem(key, data));
			}

			void Remove(const TKey& key, const TData& data) {
				q.erase(MakeQueueItem(key, data));
			}

			void DecreaseKey(const TKey& key, const TData& data, const TKey& newKey) {
				auto it = q.find(MakeQueueItem(key, data));
				if (it != q.end()) {
					q.erase(it);
				}
				Insert(newKey, data);
			}

			bool IsEmpty() const {
				return q.empty();
			}
		};
	}
}
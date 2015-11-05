#pragma once

#include <graph/queue/PriorityQueue.hpp>
#include <cassert>

namespace graph {
	namespace queue {
		template <typename TKey, typename TData>
		class DHeapQueue{
		private:
			using QueueItemType = QueueItem<TKey, TData>;
			std::vector<QueueItemType> q;

			int childrenNumber, realLength;

			void Heapify(int idx) {
				if (idx == 0)
					return;
				int leftMostChild = (idx - 1) * childrenNumber + 1;
				int largestIdx = idx;
				for (int i = leftMostChild; i < leftMostChild + childrenNumber; ++i) {
					if (i >= realLength)
						break;
					if (q[i].Key() > q[largestIdx].Key())
						largestIdx = i;
				}
				if (largestIdx != idx) {
					swap(q[idx], q[largestIdx]);
					Heapify(largestIdx);
				}

			}

			inline int GetParent(int idx) {
				return (idx - 2) / childrenNumber + 1;
			}

			void Add(QueueItemType element) {

				if (realLength > q.size() - 10)
					q.resize(3 * realLength / 2, QueueItemType(0, 0));

				q[realLength] = element;
				++realLength;

				int idx = realLength - 1;
				int parent = GetParent(idx);
				while (idx && q[parent].Key() > q[idx].Key()) {
					swap(q[parent], q[idx]);
					idx = parent;
					parent = GetParent(idx);
				}
			}

			void DeleteFromIdx(int idx) {
				q[idx] = q[realLength - 1];
				//q.pop_back();
				--realLength;
				Heapify(idx);
			}

		public:
			DHeapQueue(int dataIdSize, int d) {
				q.resize(2 * dataIdSize, QueueItemType(0, 0));
				childrenNumber = d;
				realLength = 0;
			}

			bool IsEmpty() {
				return realLength == 0;
			}

			const QueueItem<TKey, TData>& PeekMin() const {
				assert(realLength != 0);
				return *q.begin();
			}

			void DeleteMin() {
				assert(!this->IsEmpty());
				DeleteFromIdx(0);
			}

			void Insert(const TKey& key, const TData& data) {
				Add(MakeQueueItem(key, data));
			}

			void DecreaseKey(const TKey& key, const TData& data, const TKey& newKey) {
				bool success = false;
				for (int i = 0; i < realLength; i++)
					if (q[i].Key() == key && q[i].Data() == data) {
						DeleteFromIdx(i);
						success = true;
						break;
					}
				Insert(newKey, data);
			}

		};
	}
}
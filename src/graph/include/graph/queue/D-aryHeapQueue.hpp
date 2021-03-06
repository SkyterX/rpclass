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
			std::vector<int> vertexIndeces;


			int childrenNumber, realLength;

			void Heapify(int idx) {
				if (idx == 0)
					return;
				BubbleDown(idx);
			}

			void BubbleUp(int idx) {
				int parent = GetParent(idx);
				while (idx > 1 && q[parent].Key() > q[idx].Key()) {
					Swap(parent, idx);
					idx = parent;
					parent = GetParent(idx);
				}
			}

			void BubbleDown(int idx) {
				int leftMostChild = GetLeftmostChild(idx);
				int leastIdx = idx;
				for (int i = leftMostChild; i < leftMostChild + childrenNumber; ++i) {
					if (i > realLength)
						break;
					if (q[i].Key() < q[leastIdx].Key())
						leastIdx = i;
				}
				if (leastIdx != idx) {
					Swap(idx, leastIdx);
					Heapify(leastIdx);
				}
			}

			void Swap(int i1, int i2) {
				std::swap(vertexIndeces[q[i1].Data()], vertexIndeces[q[i2].Data()]);
				std::swap(q[i1], q[i2]);
			}

			inline int GetParent(int idx) {
				int parent = (idx - 2) / childrenNumber + 1;
				return parent;
			}

			inline int GetLeftmostChild(int idx) {
				int child = (idx - 1) * childrenNumber + 2;
				return child;
			}

			void Add(QueueItemType element) {
				if (realLength >= q.size()) {
					q.resize(2 * realLength, QueueItemType(0, 0));
					vertexIndeces.resize(2 * vertexIndeces.size(), 0);
				}

				++realLength;
				vertexIndeces[element.Data()] = realLength;
				q[realLength] = element;

				int idx = realLength;
				BubbleUp(idx);
			}

			void DeleteFromIdx(int idx) {
				vertexIndeces[q[idx].Data()] = -1;
				vertexIndeces[q[realLength].Data()] = idx;
				q[idx] = q[realLength];
				q[realLength] = QueueItemType(-1, -1);
				--realLength;

				BubbleDown(idx);
			}

			void UnitTests() {
				int realParent = 1;
				std::vector<int> parents;
				parents.push_back(0);
				parents.push_back(1);
				int i = 2;
				while (parents.size() <= 1000) {
					for (int j = 0; j < childrenNumber; j++)
						parents.push_back(realParent);
					realParent++;
				}

				for (int j = 1; j <= 100; j++) {
					int parent = GetParent(j);
					int leftmostChild = GetLeftmostChild(j);
					assert(parent == parents[j]);
					assert(j == parents[leftmostChild]);
					assert(j == parents[leftmostChild + childrenNumber - 1]);
				}
			}

		public:
			DHeapQueue(int dataIdSize, int d) {
				q.resize(d * dataIdSize, QueueItemType(0, 0));
				vertexIndeces.resize(dataIdSize * d, -1);
				childrenNumber = d;
				realLength = 0;

				//UnitTests();

			}

			bool Check() {
				for (int i = 1; i <= realLength; i++) {
					int leftMostChild = GetLeftmostChild(i);
					for (int j = leftMostChild; j < leftMostChild + childrenNumber; j++) {
						if (j > realLength)
							break;
						if (q[i].Key() > q[j].Key())
							return false;
					}
				}
				return true;
			}

			bool IsEmpty() {
				return realLength == 0;
			}

			const QueueItem<TKey, TData>& PeekMin() const {
				assert(realLength != 0);
				return q[1];
			}

			void DeleteMin() {
				assert(!this->IsEmpty());
				DeleteFromIdx(1);
			}

			void Insert(const TKey& key, const TData& data) {
				Add(MakeQueueItem(key, data));
			}

			void DecreaseKey(const TKey& key, const TData& data, const TKey& newKey) {
				int idx = vertexIndeces[data];
				if (idx != -1){
					q[idx] = QueueItemType(newKey, data);
					BubbleUp(idx);
				}
				else {
					Insert(newKey, data);
				}
			}

		};
	}
}
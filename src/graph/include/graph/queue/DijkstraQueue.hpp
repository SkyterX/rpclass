#pragma once
#include <cinttypes>
#include <vector>
#include <queue>
#include <functional>
#include <graph/properties.hpp>
#include <cassert>

namespace graph
{
	namespace queue
	{
		// Items cannot be reinserted.
		template <typename DistanceType, typename VertexType, typename IndexMap>
		class DijkstraQueue {
		private:
			using IndexType = typename IndexMap::value_type;
		public:
			struct DijkstraQueueItem {
				DistanceType Distance;
				IndexType VertexIndex;
				VertexType Vertex;

				DijkstraQueueItem(const DistanceType& distance, const IndexType& index, const VertexType& vertex)
					:Distance(distance),
					 VertexIndex(index),
					 Vertex(vertex) {}

				friend bool operator<(const DijkstraQueueItem& lhs, const DijkstraQueueItem& rhs) {
					return lhs.Distance < rhs.Distance ||
							(lhs.Distance == rhs.Distance && lhs.VertexIndex < rhs.VertexIndex);
				}

				friend bool operator<=(const DijkstraQueueItem& lhs, const DijkstraQueueItem& rhs) {
					return !(rhs < lhs);
				}

				friend bool operator>(const DijkstraQueueItem& lhs, const DijkstraQueueItem& rhs) {
					return rhs < lhs;
				}

				friend bool operator>=(const DijkstraQueueItem& lhs, const DijkstraQueueItem& rhs) {
					return !(lhs < rhs);
				}
			};

		private:
			using QueueType = std::priority_queue<DijkstraQueueItem, std::vector<DijkstraQueueItem>, std::greater<DijkstraQueueItem>>;
			using IterationIdType = uint32_t;

			QueueType q;
			std::vector<IterationIdType> isDeleted;
			IterationIdType iterationId;

			void SkipDeleted() {
				while (!q.empty() && isDeleted[q.top().VertexIndex] == iterationId)
					q.pop();
			}

		public:

			explicit DijkstraQueue(int dataIdSize = 0) {
				iterationId = 1;
				Resize(dataIdSize);
			}

			void Resize(int dataIdSize) {
				isDeleted.resize(dataIdSize, 0);
			}

			void Clear() {
				if (iterationId == std::numeric_limits<IterationIdType>::max()) {
					iterationId = 0;
					isDeleted.assign(isDeleted.size(), 0);
				}
				++iterationId;
				q = QueueType();
			}

			void Insert(const DistanceType& key, const VertexType& vertex, const IndexMap& index) {
				auto vertexIndex = get(index, vertex);
				assert(isDeleted[vertexIndex] < iterationId);
				q.push(DijkstraQueueItem(key, vertexIndex, vertex));
			}

			const DijkstraQueueItem& PeekMin() const {
				assert(!this->IsEmpty());
				return q.top();
			}

			void DeleteMin() {
				assert(!this->IsEmpty());
				isDeleted[q.top().VertexIndex] = iterationId;
				SkipDeleted();
			}

			void DecreaseKey(const DistanceType& newKey, const VertexType& vertex, const IndexMap& index) {
				Insert(newKey, vertex, index);
			}

			bool IsEmpty() const {
				return q.empty();
			}
		};
	}
}

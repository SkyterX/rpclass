#pragma once
#include <graph/queue/PriorityQueue.hpp>
#include <cassert>
#include <vector>

namespace graph {
	namespace queue {
		template <typename TKey, typename TDataId>
		class FibonacciHeapQueue {
		private:
			using QueueItemType = QueueItem<TKey, TDataId>;

			struct Node {
				Node()
					: Parent(nullptr), Child(nullptr),
					  Left(nullptr), Right(nullptr),
					  Degree(0), HadChildDeleted(false) { }

				void Initialize(const TKey& key, const TDataId& data) {
					this->Key = key;
					this->DataId = data;
					Left = this;
					Right = this;
				}

				TKey Key;
				TDataId DataId;
				Node* Parent;
				Node* Child;
				Node* Left;
				Node* Right;
				int Degree;
				bool HadChildDeleted;

				bool IsNull() const {
					return Left == nullptr;
				}
			};

			struct Heap {
				Node* Min;

				Heap()
					: Min(nullptr) {}

				Heap(Node* node)
					: Min(node) {
					Min->Left = Min;
					Min->Right = Min;
				}

				bool IsLess(Node* a, Node* b) const {
					return a->Key < b->Key || (a->Key == b->Key && a->DataId < b->DataId);
				}

				void Insert(Node* node) {
					InsertRoot(node);
				}

				void DecreaseKey(Node* node, TKey newKey) {
					node->Key = newKey;
					if (node->Parent == nullptr) {
						if (IsLess(node, Min))
							Min = node;
						return;
					}
					if (!IsLess(node, node->Parent))
						return;
					CascadeCut(node);
				}

				void CascadeCut(Node* node) {
					Node* parent = node->Parent;
					if (parent->Child == node)
						parent->Child = (node == node->Right) ? nullptr : node->Right;
					CutNode(node);
					node->Parent = nullptr;
					node->HadChildDeleted = false;
					InsertRoot(node);

					--parent->Degree;
					if (!parent->HadChildDeleted || parent->Parent == nullptr) {
						parent->HadChildDeleted = true;
						return;
					}
					CascadeCut(parent);
				}

				void MergeNodes(Node* a, Node* b) {
					LinkNodes(b->Left, a->Right);
					LinkNodes(a, b);
				}

				void LinkNodes(Node* a, Node* b) {
					a->Right = b;
					b->Left = a;
				}

				void InsertRoot(Node* node) {
					if (Min == nullptr) {
						Min = node;
					}
					else {
						MergeNodes(Min, node);
						if (IsLess(node, Min))
							Min = node;
					}
				}

				void CutNode(Node* node) {
					LinkNodes(node->Left, node->Right);
					node->Left = node;
					node->Right = node;
				}

				void ExtractMin() {

					if (Min->Child != nullptr) {
						// Remove parent_ptr from childs
						Node* child = Min->Child;
						do {
							child->Parent = nullptr;
							child = child->Right;
						}
						while (child != Min->Child);

						// Merge childs and and heap roots
						MergeNodes(Min, child);
						Min->Child = nullptr;
					}

					// Update heap size, roots list and min_ptr
					Node* currentMin = Min;
					Min = currentMin->Right != currentMin ? currentMin->Right : nullptr;
					CutNode(currentMin);

					// Fix structure
					Consolidate();
				}

				std::vector<Node*> roots;

				void Consolidate() {
					if (Min == nullptr)
						return;
					// Decrease number of roots
					roots.assign(50, nullptr);
					Node* root = Min;
					Node* nextRoot = root->Right;
					do {
						int d = root->Degree;
						if (roots[d] == nullptr) {
							roots[d] = root;
							root = nextRoot;
							nextRoot = root->Right;
						}
						else {
							// choose root with smallest key
							if (!IsLess(root, roots[d]))
								std::swap(root, roots[d]);
							// fix min for cycle to end
							if (roots[d] == Min) {
								Min = roots[d]->Right;
								if (roots[d] == nextRoot)
									nextRoot = Min;
							}
							// add child
							CutNode(roots[d]);
							if (root->Child == nullptr)
								root->Child = roots[d];
							else
								MergeNodes(root->Child, roots[d]);
							roots[d]->Parent = root;
							++root->Degree;
							roots[d] = nullptr;
						}
					}
					while (roots[root->Degree] != root);

					// Find min
					Node* startRoot = Min;
					root = Min->Right;
					while (root != startRoot) {
						if (IsLess(root, Min))
							Min = root;
						root = root->Right;
					}
				}
			};

			Heap heap;
			std::vector<Node> nodes;
		public:

			FibonacciHeapQueue(int dataIdSize) {
				nodes.resize(dataIdSize);
			}

			void Insert(TKey key, TDataId dataId) {
				assert(dataId < nodes.size());
				assert(nodes[dataId].IsNull());
				nodes[dataId].Initialize(key, dataId);
				heap.Insert(&nodes[dataId]);
			}

			QueueItem<TKey, TDataId> PeekMin() const {
				assert(!this->IsEmpty());
				return MakeQueueItem(heap.Min->Key, heap.Min->DataId);
			}

			void DeleteMin() {
				assert(!this->IsEmpty());
				auto minDataId = heap.Min->DataId;
				heap.ExtractMin();
				nodes[minDataId] = Node();
			}

			void DecreaseKey(const TKey& key, const TDataId& dataId, const TKey& newKey) {
				assert(dataId < nodes.size());
				assert(!nodes[dataId].IsNull());
				heap.DecreaseKey(&nodes[dataId], newKey);
			}

			bool IsEmpty() const {
				return heap.Min == nullptr;
			}
		};
	}
}

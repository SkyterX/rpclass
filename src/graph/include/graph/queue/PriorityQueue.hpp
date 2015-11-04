#pragma once

#include <tuple>

namespace graph {
	namespace queue {

		template <typename TKey, typename TData>
		class QueueItem : public std::tuple<TKey, TData> {
			using Base = std::tuple<TKey, TData>;
		public:
			QueueItem(const TKey& key, const TData& data)
				:Base(key, data) {}

			const TKey& Key() const{
				return std::get<0>(*this);
			}

			const TData& Data() const{
				return std::get<1>(*this);
			}
		};

		template <typename TKey, typename TData>
		inline QueueItem<TKey, TData> MakeQueueItem(const TKey& key, const TData& data) {
			return QueueItem<TKey, TData>(key, data);
		}
	}
}

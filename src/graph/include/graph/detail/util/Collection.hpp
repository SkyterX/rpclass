#pragma once

#include <algorithm>
#include <type_traits>
#include <boost/iterator/counting_iterator.hpp>

namespace graphUtil {
	template <typename Iterator>
	class Collection {
	private:
		Iterator first, last;
	public:
		using IteratorType = Iterator;
		using CollectionType = Collection<Iterator>;

		Collection(const Iterator& first, const Iterator& last)
			: first(first),
			  last(last) {}

		Iterator begin() const {
			return first;
		}

		Iterator end() const {
			return last;
		}

		decltype(auto) size() const {
			return end() - begin();
		}
	};

	struct SortedTag {};

	struct UnsortedTag {};

	template <typename Iterator, typename IsSorted = UnsortedTag, typename Value = typename Iterator::value_type>
	class ValueCollection : public Collection<Iterator> {
	public:
		using CollectionType = ValueCollection<Iterator, Value, IsSorted>;

		ValueCollection(const Iterator& first, const Iterator& last)
			: Collection<Iterator>(first, last) {}

		bool contains(const Value& val) const {
			return find(val) != this->end();
		}

		Iterator find(const Value& val) const {
			return find_impl<IsSorted>(val);
		}

	private:
		template <typename Tag>
		std::enable_if_t<std::is_same<UnsortedTag, Tag>::value, Iterator>
		find_impl(const Value& val) const {
			return std::find(this->begin(), this->end(), val);
		}

		template <typename Tag>
		std::enable_if_t<std::is_same<SortedTag, Tag>::value, Iterator>
		find_impl(const Value& val) const {
			Iterator it = std::lower_bound(this->begin(), this->end(), val);
			return it == this->end() || (*it) != val ? this->end() : it;
		}
	};

	template<typename Iterator>
	inline Collection<Iterator> Range(const Iterator& begin, const Iterator& end) {
		return Collection<Iterator>(begin, end);
	}

	template<typename Iterator>
	inline Collection<Iterator> Range(const std::pair<Iterator, Iterator>& iteratorRange) {
		return Collection<Iterator>(iteratorRange.first, iteratorRange.second);
	}

	template<typename Iterator, typename IsSorted = UnsortedTag, typename Value = typename Iterator::value_type>
	inline ValueCollection<Iterator, IsSorted, Value> ValuesRange(const Iterator& begin, const Iterator& end) {
		return ValueCollection<Iterator, IsSorted, Value>(begin, end);
	}

	template<typename Iterator, typename IsSorted = UnsortedTag, typename Value = typename Iterator::value_type>
	inline ValueCollection<Iterator, IsSorted, Value> ValuesRange(const std::pair<Iterator, Iterator>& iteratorRange) {
		return ValueCollection<Iterator, IsSorted, Value>(iteratorRange.first, iteratorRange.second);
	}

	template<typename Iterator, typename Value = typename Iterator::value_type>
	inline ValueCollection<Iterator, SortedTag, Value> SortedValuesRange(const Iterator& begin, const Iterator& end) {
		return ValueCollection<Iterator, SortedTag, Value>(begin, end);
	}

	template<typename Iterator, typename Value = typename Iterator::value_type>
	inline ValueCollection<Iterator, SortedTag, Value> SortedValuesRange(const std::pair<Iterator, Iterator>& iteratorRange) {
		return ValueCollection<Iterator, SortedTag, Value>(iteratorRange.first, iteratorRange.second);
	}

	inline Collection<boost::counting_iterator<int>> Range(const int begin, const int end) {
		return Collection<boost::counting_iterator<int>>(
			boost::counting_iterator<int>(begin),
			boost::counting_iterator<int>(end));
	}
}

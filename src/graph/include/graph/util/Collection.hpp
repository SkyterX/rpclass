#pragma once

#include <algorithm>
#include <type_traits>

namespace graphUtil
{
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

	template <typename Iterator, typename Value, bool IsSorted = false>
	class ValueCollection : public Collection<Iterator> {
	public:
		using CollectionType = ValueCollection<Iterator, Value, IsSorted>;

		ValueCollection(const Iterator& first, const Iterator& last)
			: Collection<Iterator>(first, last){}

		bool contains(const Value& val) const {
			return find(val) != this->end();
		}

		Iterator find(const Value& val) const {
			return find_impl<IsSorted>(val);
		}

	private:
		template <bool SortedCondition>
		std::enable_if_t<!SortedCondition, Iterator> find_impl(const Value& val) const{
			return std::find(this->begin(), this->end(), val);
		}

		template <bool Condition>
		std::enable_if_t<Condition, Iterator> find_impl(const Value& val) const{
			Iterator it = std::lower_bound(this->begin(), this->end(), val);
			return it == this->end() || (*it) != val ? this->end() : it;
		}
	};
}

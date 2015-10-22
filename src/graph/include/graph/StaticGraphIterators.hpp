#pragma once
namespace graph {
	namespace internals {
		enum EdgeDirection {
			In,
			Out
		};
	}

	template <typename AdjacencyListIterator, typename VertexDescriptor, internals::EdgeDirection Direction>
	class AdjacencyIterator
			: public boost::iterator_adaptor<
				AdjacencyIterator<AdjacencyListIterator, VertexDescriptor, Direction>,
				AdjacencyListIterator,
				VertexDescriptor> {
		typedef boost::iterator_adaptor<
			AdjacencyIterator<AdjacencyListIterator, VertexDescriptor, Direction>,
			AdjacencyListIterator,
			VertexDescriptor> BaseType;
	public:
		explicit AdjacencyIterator(const AdjacencyListIterator& p)
			: BaseType(p) {}

	private:
		friend class boost::iterator_core_access;
		friend class edge_iterator;

		VertexDescriptor& dereference() const {
			return dereference_impl<Direction>();
		}

		template <internals::EdgeDirection EdgeDirection>
		inline std::enable_if_t<EdgeDirection == internals::EdgeDirection::In, VertexDescriptor>& dereference_impl() const {
			return (*this->base_reference())->source;
		}

		template <internals::EdgeDirection EdgeDirection>
		inline std::enable_if_t<EdgeDirection == internals::EdgeDirection::Out, VertexDescriptor>& dereference_impl() const {
			return (*this->base_reference())->target;
		}
	};

	template <typename AdjacencyListIterator, typename EdgeDescriptor>
	class EdgeIterator
			: public boost::iterator_adaptor<
				EdgeIterator<AdjacencyListIterator, EdgeDescriptor>,
				AdjacencyListIterator,
				EdgeDescriptor,
				boost::use_default,
				EdgeDescriptor> {
		typedef boost::iterator_adaptor<
			EdgeIterator<AdjacencyListIterator, EdgeDescriptor>,
			AdjacencyListIterator,
			EdgeDescriptor,
			boost::use_default,
			EdgeDescriptor> BaseType;
	public:
		explicit EdgeIterator(const AdjacencyListIterator& p)
			: BaseType(p) {}

	private:
		friend class boost::iterator_core_access;

		EdgeDescriptor dereference() const {
			return EdgeDescriptor(**this->base_reference());
		}
	};
}

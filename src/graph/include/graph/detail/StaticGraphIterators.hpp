#pragma once
namespace graph {
	namespace internals {
		enum EdgeDirection {
			In,
			Out
		};
	}

	template <typename AdjacencyListIterator, typename VertexDescriptor>
	class AdjacencyIterator
			: public boost::iterator_adaptor<
				AdjacencyIterator<AdjacencyListIterator, VertexDescriptor>,
				AdjacencyListIterator,
				const VertexDescriptor> {
		using BaseType = boost::iterator_adaptor<
			AdjacencyIterator<AdjacencyListIterator, VertexDescriptor>,
			AdjacencyListIterator,
			const VertexDescriptor>;
	public:
		explicit AdjacencyIterator(const AdjacencyListIterator& p)
			: BaseType(p) {}

	private:
		friend class boost::iterator_core_access;

		const VertexDescriptor& dereference() const {
			return this->base_reference()->target;
		}
	};

	template <typename AdjacencyListIterator, typename EdgeDescriptor, internals::EdgeDirection Direction>
	class EdgeIterator
			: public boost::iterator_adaptor<
				EdgeIterator<AdjacencyListIterator, EdgeDescriptor, Direction>,
				AdjacencyListIterator,
				EdgeDescriptor,
				boost::use_default,
				EdgeDescriptor> {
		using BaseType = boost::iterator_adaptor<
			EdgeIterator<AdjacencyListIterator, EdgeDescriptor, Direction>,
			AdjacencyListIterator,
			EdgeDescriptor,
			boost::use_default,
			EdgeDescriptor>;
		using VertexType = typename EdgeDescriptor::VertexType;
	public:
		explicit EdgeIterator(const VertexType& v, const AdjacencyListIterator& p)
			: BaseType(p), source(v) {}

	private:
		friend class boost::iterator_core_access;

		EdgeDescriptor dereference() const {
			return dereference_impl<Direction>();
		}

		template <internals::EdgeDirection EdgeDirection>
		inline std::enable_if_t<EdgeDirection == internals::EdgeDirection::In, EdgeDescriptor> dereference_impl() const {
			return EdgeDescriptor(this->base_reference()->target, source, this->base_reference()->properties);
		}

		template <internals::EdgeDirection EdgeDirection>
		inline std::enable_if_t<EdgeDirection == internals::EdgeDirection::Out, EdgeDescriptor> dereference_impl() const {
			return EdgeDescriptor(source, this->base_reference()->target, this->base_reference()->properties);
		}

		VertexType source;
	};
}

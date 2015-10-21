#pragma once

// adjacency_iterator
template <typename AdjacencyListIterator, typename VertexDescriptor>
class AdjacencyIterator
	: public boost::iterator_adaptor<
	AdjacencyIterator<AdjacencyListIterator, VertexDescriptor>,
	AdjacencyListIterator,
	VertexDescriptor> {
	typedef boost::iterator_adaptor<
		AdjacencyIterator<AdjacencyListIterator, VertexDescriptor>,
		AdjacencyListIterator,
		VertexDescriptor> BaseType;
public:
	explicit AdjacencyIterator(const AdjacencyListIterator& p, bool isForInEdges)
		: BaseType(p), isForInEdges(isForInEdges) {}

private:
	friend class boost::iterator_core_access;
	friend class edge_iterator;

	VertexDescriptor& dereference() const {
		return isForInEdges ? (*this->base_reference())->source : (*this->base_reference())->target;
	}

	bool isForInEdges;
};

// edge_iterator
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
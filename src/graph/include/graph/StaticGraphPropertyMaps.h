#pragma once
#include "static_graph.hpp"
#include "properties.hpp"

// VertexPropertyMap
namespace graph
{
	class StaticGraph::VertexPropertyMap {
	private:
		friend class StaticGraph;

		StaticGraph& graph;

		explicit VertexPropertyMap(StaticGraph& graph)
			: graph(graph) {}

	public:
		using key_type = vertex_descriptor;
		using value_type = vertex_properties_stub;
		using reference = value_type&;
		using category = boost::read_write_property_map_tag;

		value_type& get(key_type& key) const {
			return graph.vertices[key].properties;
		}

		void put(const key_type& key, const value_type& value) const {
			graph.vertices[key].properties = value;
		}
	};


	inline StaticGraph::VertexPropertyMap::value_type& get(
		const StaticGraph::VertexPropertyMap& pm,
		StaticGraph::VertexPropertyMap::key_type& key) {
		return pm.get(key);
	}

	inline void put(
		const StaticGraph::VertexPropertyMap& pm,
		const StaticGraph::VertexPropertyMap::key_type& key,
		const StaticGraph::VertexPropertyMap::value_type& value) {
		pm.put(key, value);
	}

	//	template<>
	//	struct property_map<StaticGraph, vertex_bundle_t> {
	//		using type = StaticGraph::VertexPropertyMap;
	//	};
	//
	//	template<>
	//	inline property_map<StaticGraph, vertex_bundle_t>::type
	//		get<StaticGraph>(const vertex_bundle_t&, StaticGraph& graph) {
	//		return graph.GetVertexPropertyMap();
	//	}
}

// EdgePropertyMap
namespace graph
{
	class StaticGraph::EdgePropertyMap {
	public:
		using key_type = edge_descriptor;
		using value_type = edge_properties_stub;
		using reference = value_type&;
		using category = boost::read_write_property_map_tag;

		value_type& get(key_type& key) const {
			return *key.properties;
		}

		void put(const key_type& key, const value_type& value) const {
			*key.properties = value;
		}
	};

	inline StaticGraph::EdgePropertyMap::value_type& get(
		const StaticGraph::EdgePropertyMap& pm,
		StaticGraph::EdgePropertyMap::key_type& key) {
		return pm.get(key);
	}

	inline void put(
		const StaticGraph::EdgePropertyMap& pm,
		const StaticGraph::EdgePropertyMap::key_type& key,
		const StaticGraph::EdgePropertyMap::value_type& value) {
		pm.put(key, value);
	}

	//	template<>
	//	struct property_map<StaticGraph, edge_bundle_t> {
	//		using type = StaticGraph::EdgePropertyMap;
	//	};
	//
	//	template<>
	//	inline property_map<StaticGraph, edge_bundle_t>::type
	//		get<StaticGraph>(const edge_bundle_t&, StaticGraph& graph) {
	//		return graph.GetEdgePropertyMap();
	//	}
}
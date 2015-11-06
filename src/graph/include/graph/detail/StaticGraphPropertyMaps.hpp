#pragma once

#include "graph/graph.hpp"
#include <boost/property_map/property_map.hpp>

// VertexPropertyMap
namespace graph {
	template <typename VertexProperties, typename Graph>
	class VertexPropertyMap {
	private:
		Graph* graph;
	public:
		explicit VertexPropertyMap(Graph* graph)
			: graph(graph) {}

		VertexPropertyMap()
			: graph(nullptr) {}

		using key_type = typename graph_traits<Graph>::vertex_descriptor;
		using value_type = VertexProperties;
		using reference = value_type&;
		using category = boost::read_write_property_map_tag;

		reference get(const key_type& key) const {
			return graph->vertices[key].properties;
		}

		void put(const key_type& key, const value_type& value) const {
			graph->vertices[key].properties = value;
		}
	};

	template <typename VertexProperties, typename Graph>
	inline typename VertexPropertyMap<VertexProperties, Graph>::reference get(
		const VertexPropertyMap<VertexProperties, Graph>& pm,
		const typename VertexPropertyMap<VertexProperties, Graph>::key_type& key) {
		return pm.get(key);
	}

	template <typename VertexProperties, typename Graph>
	inline void put(
		VertexPropertyMap<VertexProperties, Graph>& pm,
		const typename VertexPropertyMap<VertexProperties, Graph>::key_type& key,
		const typename VertexPropertyMap<VertexProperties, Graph>::reference value) {
		pm.put(key, value);
	}

}

// EdgePropertyMap
namespace graph {
	template <typename EdgeProperties, typename Graph>
	class EdgePropertyMap {
	public:
		using key_type = typename graph_traits<Graph>::edge_descriptor;
		using value_type = EdgeProperties;
		using reference = value_type&;
		using category = boost::read_write_property_map_tag;

		value_type& get(const key_type& key) const {
			return *key.properties;
		}

		void put(const key_type& key, const value_type& value) const {
			*key.properties = value;
		}
	};

	template <typename EdgeProperties, typename Graph>
	inline typename EdgePropertyMap<EdgeProperties, Graph>::value_type& get(
		const EdgePropertyMap<EdgeProperties, Graph>& pm,
		const typename EdgePropertyMap<EdgeProperties, Graph>::key_type& key) {
		return pm.get(key);
	}

	template <typename EdgeProperties, typename Graph>
	inline void put(
		EdgePropertyMap<EdgeProperties, Graph>& pm,
		const typename EdgePropertyMap<EdgeProperties, Graph>::key_type& key,
		const typename EdgePropertyMap<EdgeProperties, Graph>::value_type& value) {
		pm.put(key, value);
	}
}

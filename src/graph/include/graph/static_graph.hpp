#pragma once

#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>


namespace graph {

    template <typename BundledVertexProperties, typename BundledEdgeProperties>
    using StaticGraph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::bidirectionalS,
        BundledVertexProperties, BundledEdgeProperties>;
  
    template <typename BundledVertexProperties, typename BundledEdgeProperties>
    struct graph_traits<StaticGraph<BundledVertexProperties, BundledEdgeProperties>>
        : boost::graph_traits<StaticGraph<BundledVertexProperties, BundledEdgeProperties>> {};

    template <typename BundledVertexProperties, typename BundledEdgeProperties>
    struct property_map<
        StaticGraph<BundledVertexProperties, BundledEdgeProperties>, 
        vertex_bundle_t,
        void > {
        using type = typename boost::property_map<
            StaticGraph<BundledVertexProperties, BundledEdgeProperties>, 
            boost::vertex_bundle_t>::type;
    };
    template <typename BundledVertexProperties, typename BundledEdgeProperties>
    struct property_map< 
        StaticGraph<BundledVertexProperties, BundledEdgeProperties>, 
        edge_bundle_t,
        void > {
        using type = typename boost::property_map<
            StaticGraph<BundledVertexProperties, BundledEdgeProperties>, 
            boost::edge_bundle_t>::type;
    };

    template <typename BundledVertexProperties, typename BundledEdgeProperties>
    inline typename property_map<
        StaticGraph<BundledVertexProperties, BundledEdgeProperties>,
        vertex_bundle_t>::type
        get(const vertex_bundle_t&,
            StaticGraph<BundledVertexProperties, BundledEdgeProperties>& graph) {
        return boost::get(boost::vertex_bundle, graph);
    };

    template <typename BundledVertexProperties, typename BundledEdgeProperties>
    inline typename property_map<
        StaticGraph<BundledVertexProperties, BundledEdgeProperties>,
        edge_bundle_t>::type
        get(const edge_bundle_t&,
            StaticGraph<BundledVertexProperties, BundledEdgeProperties>& graph) {
        return boost::get(boost::edge_bundle, graph);
    };

};

// Supress boost get functions instantiation by supressing instantiation of boost::property_map template.

namespace boost {

template <typename Graph>
struct property_map<Graph, ::graph::vertex_bundle_t, void> {};

template <typename Graph>
struct property_map<Graph, ::graph::edge_bundle_t, void> {};

}


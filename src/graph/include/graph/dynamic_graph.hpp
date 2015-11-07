#pragma once

#include <vector>
#include <utility>
#include <graph/graph.hpp>
#include <graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>


namespace graph {

    template <typename VP = Properties<>,
        typename EP = Properties<>,
        typename BundledGraphProperties = Properties<>>
    class DynamicGraph:public boost::adjacency_list<
        boost::vecS, boost::vecS, boost::bidirectionalS,
        VP, EP>{
    public:
        using BoostBase = boost::adjacency_list<
            boost::vecS, boost::vecS, boost::bidirectionalS,
            VP, EP>;
        struct traversal_category : bidirectional_graph_tag, vertex_list_graph_tag, BoostBase::traversal_category {};
        DynamicGraph(){};
        DynamicGraph(BoostBase& base) :BoostBase(base) {};
        DynamicGraph(BoostBase&& base) :BoostBase(base) {};

        DynamicGraph(std::vector<std::pair<size_t, size_t>>::iterator begin,
            std::vector<std::pair<size_t, size_t>>::iterator end,
            size_t n, size_t m = 0):BoostBase(begin, end, n, m) {};

        template <typename Iterator>
        DynamicGraph(Iterator begin, Iterator end, size_t n, size_t m = 0):BoostBase() {
            using namespace std;
            vector<pair<size_t, size_t>> edges;
            edges.reserve(m);
            vector<EP> properties;
            properties.reserve(m);
            for (Iterator it = begin; it != end; ++it) {
                edges.push_back(it->first);
                EP p;
                p = make_properties(it->second);
                properties.push_back(p);
            };
            static_cast<BoostBase*>(this)->operator=(BoostBase(edges.begin(), edges.end(), properties.begin(), n, m));
        }
    };
  
    template <typename VP, typename EP, typename GP>
    struct property_map<
        DynamicGraph<VP, EP, GP>, 
        graph::vertex_bundle_t,
        void > {
        using type = typename boost::property_map<
            typename DynamicGraph<VP, EP, GP>::BoostBase, 
            boost::vertex_bundle_t>::type;
    };
    template <typename VP, typename EP, typename GP>
    struct property_map< 
        DynamicGraph<VP, EP, GP>,
        graph::edge_bundle_t,
        void > {
        using type = typename boost::property_map<
            typename DynamicGraph<VP, EP, GP>::BoostBase,
            boost::edge_bundle_t>::type;
    };
    template <typename VP, typename EP, typename GP>
    struct property_map<
        DynamicGraph<VP, EP, GP>,
        graph::vertex_index_t,
        void > {
        using type = typename boost::property_map<
            typename DynamicGraph<VP, EP, GP>::BoostBase,
            boost::vertex_index_t>::type;
    };

    template <typename VP, typename EP, typename GP>
    inline typename property_map<
        DynamicGraph<VP, EP, GP>,
        vertex_bundle_t>::type
        get(const vertex_bundle_t&,
            DynamicGraph<VP, EP, GP>& graph) {
        return boost::get(boost::vertex_bundle, graph);
    };

    template <typename VP, typename EP, typename GP>
    inline typename property_map<
        DynamicGraph<VP, EP, GP>,
        edge_bundle_t>::type
        get(const edge_bundle_t&,
            DynamicGraph<VP, EP, GP>& graph) {
        return boost::get(boost::edge_bundle, graph);
    };

    template <typename VP, typename EP, typename GP>
    inline typename property_map<
        DynamicGraph<VP, EP, GP>,
        vertex_index_t>::type
        get(const vertex_index_t&,
            DynamicGraph<VP, EP, GP>& graph) {
        return boost::get(boost::vertex_index, graph);
    };

};

// Supress boost get functions instantiation by supressing instantiation of boost::property_map template.

namespace boost {

template <typename G>
struct property_map<G, ::graph::edge_bundle_t, void>;

template <typename G>
struct property_map<G, ::graph::vertex_bundle_t, void>;

};


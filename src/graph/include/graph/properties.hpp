#pragma once
#include <tuple>
#include <type_traits>
#include <graph/graph.hpp>


namespace graph {

// Property map tags

    struct readable_property_map_tag { };

    struct writable_property_map_tag { };

    struct read_write_property_map_tag :
        public readable_property_map_tag,
        public writable_property_map_tag { };

    struct lvalue_property_map_tag :
        public read_write_property_map_tag { };

// Stores a property of a Value type marked by the Tag tag
    template <typename Tag, typename Value>
    struct Property :public std::tuple<Value> {
        using Base = std::tuple<Value>;
        using tag_type = Tag;
        Property(const Value& v) :Base(v) {};
        Property() :Base() {};
    };

// Stores several properties 
    template <typename... Ps>
    struct Properties :public std::tuple<Ps...> {
        using Base = std::tuple<Ps...>;
    };


    namespace detail {
        template <typename T, typename U>
        using IsSame_t = typename std::is_same<T, U>::type;

// Finds an index of a Property in Properties or returns void
        template <typename Tag, typename Props, size_t I = 0>
        struct FindPropertyByTag;

        template <typename Tag, typename Props, size_t I = 0>
        using FindPropertyByTag_t = typename FindPropertyByTag<Tag, Props, I>::type;

        template < typename Tag, size_t I, typename P, typename... Ps >
        struct FindPropertyByTag < Tag, Properties<P, Ps...>, I > {
            using type = std::conditional_t <
                IsSame_t<Tag, typename P::tag_type>::value, 
                std::integral_constant<size_t, I>, 
                FindPropertyByTag_t<Tag, Properties<Ps...>, I + 1 >
            >;
        };

        template < typename Tag, size_t I >
        struct FindPropertyByTag < Tag, Properties < >, I > {
            using type = void;
        };
        
        template <typename T>
        class HasType {
            template < typename C, typename = typename C::type>
            static std::true_type check(const C&) {};
            static std::false_type check(...) {};
        public:
            using value_type = bool;
            using type = decltype(check(std::declval<T>()));
            static const bool value = type::value;

        };

// Usefull predicates (to replace with concepts at some point)
        template < typename PropertyMap >
        class IsPropertyMap {
            template < typename C, typename = typename C::category, typename = typename C::value_type
                , typename = typename C::key_type, typename = typename C::reference >
                static std::true_type check(const C&) {};
            static std::false_type check(...) {};
        public:
            using type = decltype(check(std::declval<PropertyMap>()));
            static const bool value = type::value;
        };
    };

// Functions to access properties by tag (correspond to tuple's getters)
    template <typename Tag, typename... Ps>
    inline decltype(auto) get(Properties<Ps...>& props) {
        using PropertiesTuple = typename Properties<Ps...>::Base;
        using Index = detail::FindPropertyByTag_t<Tag, Properties<Ps...>>;
        using PTuple = typename std::tuple_element<Index::value, PropertiesTuple>::type::Base;
        return std::get<0>(static_cast<PTuple&>(
            std::get<Index::value>(static_cast<PropertiesTuple&>(props))));
    };

    template <typename Tag, typename... Ps>
    inline decltype(auto) get(Properties<Ps...>&& props) {
        using PropertiesTuple = typename Properties<Ps...>::Base;
        using Index = detail::FindPropertyByTag_t<Tag, Properties<Ps...>>;
        using PTuple = typename std::tuple_element<Index::value, PropertiesTuple>::type::Base;
        return std::get<0>(static_cast<PTuple&&>(
            std::get<Index::value>(static_cast<PropertiesTuple&&>(props))));
    };

    template <typename Tag, typename... Ps>
    inline decltype(auto) get(const Properties<Ps...>& props) {
        using PropertiesTuple = typename Properties<Ps...>::Base;
        using Index = detail::FindPropertyByTag_t<Tag, Properties<Ps...>>;
        using PTuple = typename std::tuple_element<Index::value, PropertiesTuple>::type::Base;
        return std::get<0>(static_cast<const PTuple&>(
            std::get<Index::value>(static_cast<const PropertiesTuple&>(props))));
    };

// Generic property traits class
    template <typename PropertyMap, typename EnableIf = typename PropertyMap::category>
    struct property_traits {
            using value_type = typename PropertyMap::value_type;
            using reference = typename PropertyMap::reference;
            using key_type = typename PropertyMap::key_type;
            using category = typename PropertyMap::category;
    };

// Restricts default property traits to property maps only
    //template <typename PropertyMap>
    //struct property_traits<PropertyMap, std::enable_if_t<detail::IsPropertyMap<PropertyMap>::value>> {
    //    using value_type = typename PropertyMap::value_type;
    //    using reference = typename PropertyMap::reference;
    //    using key_type = typename PropertyMap::key_type;
    //    using category = typename PropertyMap::category;
    //};

// Generic property map type. Is to specialize for each property map in the user code
    template <typename Graph, typename Tag, typename EnableIf = void>
    struct property_map {};

    template <typename Graph, typename Tag>
    using property_map_t = typename property_map<Graph, Tag>::type;


// Generic property map functions declarations. Are to define for each property map in the user code
    //template <typename PropertyMap>
    //typename property_traits<PropertyMap>::reference
    //    get(const PropertyMap&,
    //        const typename property_traits<PropertyMap>::key_type&);

    //template <typename PropertyMap>
    //void put(PropertyMap&,
    //    const typename property_traits<PropertyMap>::key_type&,
    //    const typename property_traits<PropertyMap>::value_type&);



// Bundled properties section 
    struct vertex_bundle_t {};
    struct edge_bundle_t {};
    struct vertex_index_t {};

    template<typename Graph>
    struct vertex_bundle_type {
        using type = typename Graph::vertex_bundled;
    };
    template <typename Graph>
    using vertex_bundle_type_t = typename vertex_bundle_type<Graph>::type;

    template<typename Graph>
    struct edge_bundle_type {
        using type = typename Graph::edge_bundled;
    };
    template <typename Graph>
    using edge_bundle_type_t = typename edge_bundle_type<Graph>::type;


    template <typename Graph>
    typename property_map<Graph, vertex_bundle_t>::type get(const vertex_bundle_t&, Graph&);

    template <typename Graph>
    typename property_map<Graph, edge_bundle_t>::type get(const edge_bundle_t&, Graph&);


    namespace detail {


// A property map type to map subproperties of internal properties 
        template <typename Tag, typename BaseBundledPM>
        class BundledSubPropertyMap: public BaseBundledPM {
            using Props = typename property_traits<BaseBundledPM>::value_type;
            using Index = detail::FindPropertyByTag_t<Tag, Props>;
        public:
            using value_type = typename std::tuple_element<0,typename std::tuple_element<Index::value, typename Props::Base>::type::Base>::type;
            using reference = value_type&;
            using key_type = typename BaseBundledPM::key_type;
            using category = typename BaseBundledPM::category;
            BundledSubPropertyMap(BaseBundledPM&& base) :BaseBundledPM(base) {};
            BundledSubPropertyMap(BaseBundledPM& base) :BaseBundledPM(base) {};
            BundledSubPropertyMap():BaseBundledPM() {};
        };

        template <typename Graph>
        struct HasBundledVertexProperty:HasType<vertex_bundle_type<Graph>>{};

        template <typename Graph>
        struct HasBundledEdgeProperty:HasType<edge_bundle_type<Graph>>{};


        template <typename Graph, typename Tag, typename BundleTag, typename EnableIf = void>
        struct IsTagNotInBundleTag: std::true_type {};
        
        template <typename Graph, typename Tag>
        struct IsTagNotInBundleTag<Graph,Tag,vertex_bundle_t,
            std::enable_if_t<HasBundledVertexProperty<Graph>::value>>
            :std::is_same<
                void,
                detail::FindPropertyByTag_t<
                    Tag, 
                    vertex_bundle_type_t<Graph>>
            >{};

        template <typename Graph, typename Tag>
        struct IsTagNotInBundleTag<Graph,Tag, edge_bundle_t,
            std::enable_if_t<HasBundledEdgeProperty<Graph>::value>>
            :std::is_same<
                void,
                detail::FindPropertyByTag_t<
                    Tag, 
                    edge_bundle_type_t<Graph>>
            >{};

        template <typename Graph, typename BundleTag, typename Tag, typename EnableIf = void>
        struct IsGraphAndNotInBundleTag :std::true_type {};
        template <typename Graph, typename BundleTag, typename Tag>
        struct IsGraphAndNotInBundleTag < Graph, BundleTag, Tag,
            std::enable_if_t < IsGraph<Graph>::value >>
            : IsTagNotInBundleTag<Graph, BundleTag, Tag>{};
    };

// Property map type generators for internal subproperties
    template <class Graph, class Tag>
    struct property_map<Graph, Tag, 
        std::enable_if_t<!detail::IsGraphAndNotInBundleTag<Graph, Tag, vertex_bundle_t>::value>> {
        using type = detail::BundledSubPropertyMap<Tag, typename property_map<Graph, vertex_bundle_t>::type>;
    };

    template <class Graph, class Tag>
    struct property_map<Graph, Tag, 
        std::enable_if_t<!detail::IsGraphAndNotInBundleTag<Graph, Tag, edge_bundle_t>::value>> {
        using type = detail::BundledSubPropertyMap<Tag, typename property_map<Graph, edge_bundle_t>::type>;
    };

// Property map functions for internal subproperties
    template <typename Graph, typename Tag> 
    inline typename property_map<Graph, Tag>::type get(Tag, Graph& graph, 
        std::enable_if_t<!detail::IsGraphAndNotInBundleTag<Graph, Tag, vertex_bundle_t>::value>* = nullptr) {
        using VertexBundledPM = typename property_map<Graph, vertex_bundle_t>::type;                
        return detail::BundledSubPropertyMap<Tag, VertexBundledPM>(
            get(vertex_bundle_t(), graph));
    };

    template <typename Graph, typename Tag>
    inline typename property_map<Graph, Tag>::type get(Tag, Graph& graph,
        std::enable_if_t<!detail::IsGraphAndNotInBundleTag<Graph, Tag, edge_bundle_t>::value>* = nullptr) {
        using EdgeBundledPM = typename property_map<Graph, edge_bundle_t>::type;
        return detail::BundledSubPropertyMap<Tag, EdgeBundledPM>(
            get(edge_bundle_t(), graph));
    };
 
    template <typename Tag, typename BaseBundledPM>
    inline typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::reference get(
        const detail::BundledSubPropertyMap<Tag, BaseBundledPM>& pMap,
        const typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::key_type& key) {        
        return get<Tag>(get(static_cast<const BaseBundledPM&>(pMap), key));
    };

    template <typename Tag, typename BaseBundledPM>
    inline void  put(detail::BundledSubPropertyMap<Tag, BaseBundledPM>& pMap,
        const typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::key_type& key,
        const typename detail::BundledSubPropertyMap<Tag, BaseBundledPM>::value_type& value) {
        auto tpl = get(static_cast<BaseBundledPM&>(pMap), key);
        get<Tag>(tpl) = value;
        put(static_cast<BaseBundledPM&>(pMap), key, tpl);
    };
}